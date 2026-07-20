#include "param.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../hislip/scpi_transport.h"
#include "cJSON.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include "../ring/ringbuffer.h"
#include "../awg/awg.h"



int g_device_model = DEVICE_MODEL_UNKNOW;

static MappingTable g_cvmap;

static ParamItem* g_param_items = NULL;
static ParamItem* g_last_param_items = NULL;
static int g_param_count = 0;

static SystemConfig configs;

static char* current_path = NULL;


int get_device_model(void)
{
	return g_device_model;
}

ParamItem* get_param_items(void)
{
    return g_param_items;
}


int get_param_count(void)
{
    return g_param_count;
}

void get_system_param(SystemConfig **sc)
{
	assert(sc != NULL);

	*sc = &configs;
}

void get_cv_maptable(MappingTable **cvmt)
{
	assert(cvmt != NULL);

	*cvmt = &g_cvmap;
}

int set_mac_address(const char* iface, const char* mac)
{

    char cmd[128];
    snprintf(cmd, sizeof(cmd), "ifconfig %s down", iface);


    int ret = system(cmd);
    if (ret == -1) {
        perror("system");
        return -1;
    }


    snprintf(cmd, sizeof(cmd), "ifconfig %s hw ether %s", iface, mac);
    ret = system(cmd);
    if (ret == -1) {
        perror("system");
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "ifconfig %s up", iface);
    ret = system(cmd);
    if (ret == -1) {
        perror("system");
        return -1;
    }

    printf("MAC Addr Set Success: %s -> %s\n", iface, mac);
    return 0;
}

int set_param_by_name(int objectindex, const char* name, int index, ParamValue new_value)
{
	ParamItem *pobject = NULL;

	if(objectindex == PARAM_CURRENT) {
		pobject = g_param_items;
	}else if(objectindex == PARAM_LAST) {
		pobject = g_last_param_items;
	}

	if (!pobject || !name || index < 0) return -1;

    for (int i = 0; i < g_param_count; ++i) {
        if (strcmp(pobject[i].name, name) == 0) {
            if (index >= pobject[i].param_count) return -2;

            ParamValue* old = &pobject[i].values[index];


            if (old->type == PARAM_STRING && old->value.str) {
                free(old->value.str);
                old->value.str = NULL;
            }


            if (old->type != new_value.type){
				printf("[%s][%d]:Local Param Type Error...\r\n", __FUNCTION__, __LINE__);
				return -3;
			}

            switch (new_value.type) {
                case PARAM_INT32:  old->value.i32 = new_value.value.i32; break;
                case PARAM_UINT32: old->value.u32 = new_value.value.u32; break;
                case PARAM_INT64:  old->value.i64 = new_value.value.i64; break;
                case PARAM_UINT64: old->value.u64 = new_value.value.u64; break;
                case PARAM_DOUBLE: old->value.d   = new_value.value.d;   break;
                case PARAM_STRING:
                    if (new_value.value.str)
                        old->value.str = strdup(new_value.value.str);
                    break;
                default:
                    return -4;
            }

            return 0;
        }
    }

    return -5;
}

int get_param_by_name(int objectindex, const char* name, int index, ParamValue* out_value)
{
	ParamItem *pobject = NULL;

	if(objectindex == PARAM_CURRENT) {
		pobject = g_param_items;
	}else if(objectindex == PARAM_LAST) {
		pobject = g_last_param_items;
	}

	if (!pobject || !name || index < 0 || !out_value) return -1;

    for (int i = 0; i < g_param_count; ++i) {
        if (strcmp(pobject[i].name, name) == 0) {
            if (index >= pobject[i].param_count) return -2;

            ParamValue* src = &pobject[i].values[index];
            out_value->type = src->type;
            switch (src->type) {
                case PARAM_INT32:  out_value->value.i32 = src->value.i32; break;
                case PARAM_UINT32: out_value->value.u32 = src->value.u32; break;
                case PARAM_INT64:  out_value->value.i64 = src->value.i64; break;
                case PARAM_UINT64: out_value->value.u64 = src->value.u64; break;
                case PARAM_DOUBLE: out_value->value.d   = src->value.d;   break;
                case PARAM_STRING:
                    if (src->value.str)
                        out_value->value.str = strdup(src->value.str);
                    else
                        out_value->value.str = NULL;
                    break;
                default:
                    return -3;
            }

            return 0;
        }
    }

    return -4;
}


ParamItem* load_params_from_json(const char* filepath, int* out_count)
{
    FILE* fp = fopen(filepath, "rb");
    if (!fp) {
        printf("Failed to open JSON file");
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* content = (char*)malloc(size + 1);
    fread(content, 1, size, fp);
    content[size] = '\0';
    fclose(fp);

    cJSON* root = cJSON_Parse(content);

    free(content);
    if (!root || !cJSON_IsArray(root)) {
        printf("Invalid JSON format\n");
        return NULL;
    }

    int count = cJSON_GetArraySize(root);
    if (count > MAX_PARAM_ITEMS) count = MAX_PARAM_ITEMS;

    ParamItem* items = (ParamItem*)calloc(count, sizeof(ParamItem));
	ParamItem* items_last = (ParamItem*)calloc(count, sizeof(ParamItem));
    for (int i = 0; i < count; ++i) {
        cJSON* obj = cJSON_GetArrayItem(root, i);
        if (!cJSON_IsObject(obj)) continue;

        items[i].name = strdup(cJSON_GetObjectItem(obj, "name")->valuestring);
        items[i].scpi_cmd = strdup(cJSON_GetObjectItem(obj, "scpiCmd")->valuestring);
		items_last[i].name = strdup(cJSON_GetObjectItem(obj, "name")->valuestring);
        items_last[i].scpi_cmd = strdup(cJSON_GetObjectItem(obj, "scpiCmd")->valuestring);

        cJSON* types = cJSON_GetObjectItem(obj, "type");
        cJSON* params = cJSON_GetObjectItem(obj, "params");

        int param_count = cJSON_GetArraySize(types);
        if (param_count > cJSON_GetArraySize(params)) param_count = cJSON_GetArraySize(params);

        items[i].param_count = param_count;
        items[i].values = (ParamValue*)calloc(param_count, sizeof(ParamValue));
		items_last[i].param_count = param_count;
        items_last[i].values = (ParamValue*)calloc(param_count, sizeof(ParamValue));

        for (int j = 0; j < param_count; ++j) {
            int type = cJSON_GetArrayItem(types, j)->valueint;
            cJSON* val = cJSON_GetArrayItem(params, j);
            items[i].values[j].type = (ParamType)type;
			items_last[i].values[j].type = (ParamType)type;

            switch (type) {
                case PARAM_INT32:
                    items[i].values[j].value.i32 = val->valueint;
					items_last[i].values[j].value.i32 = val->valueint;
                    break;
                case PARAM_UINT32:
                    items[i].values[j].value.u32 = (unsigned int)val->valueint;
					items_last[i].values[j].value.u32 = (unsigned int)val->valueint;
                    break;
                case PARAM_INT64:
                    items[i].values[j].value.i64 = (long long)val->valueint;//atoll(val->valuestring);
                    items_last[i].values[j].value.i64 = (long long)val->valueint;//atoll(val->valuestring);
                    break;
                case PARAM_UINT64:
                    items[i].values[j].value.u64 = (unsigned long long)val->valueint;//strtoull(val->valuestring, NULL, 10);
                    items_last[i].values[j].value.u64 = (unsigned long long)val->valueint;//strtoull(val->valuestring, NULL, 10);
                    break;
                case PARAM_DOUBLE:
                    items[i].values[j].value.d = val->valuedouble;
					items_last[i].values[j].value.d = val->valuedouble;
                    break;
                case PARAM_STRING:
                    items[i].values[j].value.str = strdup(val->valuestring);
					items_last[i].values[j].value.str = strdup(val->valuestring);
                    break;
            }
        }
    }

    cJSON_Delete(root);
    if (out_count) *out_count = count;

    g_param_items = items;
    g_param_count = count;
	g_last_param_items = items_last;

    return items;
}


int save_params_to_json(const char* filepath, const ParamItem* items, int count)
{
    cJSON* root = cJSON_CreateArray();

    for (int i = 0; i < count; ++i) {
        cJSON* obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "name", items[i].name);
        cJSON_AddStringToObject(obj, "scpiCmd", items[i].scpi_cmd);

        cJSON* types = cJSON_CreateArray();
        cJSON* params = cJSON_CreateArray();

        for (int j = 0; j < items[i].param_count; ++j) {
            cJSON_AddItemToArray(types, cJSON_CreateNumber(items[i].values[j].type));

            switch (items[i].values[j].type) {
                case PARAM_INT32:
                    cJSON_AddItemToArray(params, cJSON_CreateNumber(items[i].values[j].value.i32));
                    break;
                case PARAM_UINT32:
                    cJSON_AddItemToArray(params, cJSON_CreateNumber(items[i].values[j].value.u32));
                    break;
                case PARAM_INT64: {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%lld", items[i].values[j].value.i64);
                    cJSON_AddItemToArray(params, cJSON_CreateString(buf));
                    break;
                }
                case PARAM_UINT64: {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%llu", items[i].values[j].value.u64);
                    cJSON_AddItemToArray(params, cJSON_CreateString(buf));
                    break;
                }
                case PARAM_DOUBLE:
                    cJSON_AddItemToArray(params, cJSON_CreateNumber(items[i].values[j].value.d));
                    break;
                case PARAM_STRING:
                    cJSON_AddItemToArray(params, cJSON_CreateString(items[i].values[j].value.str));
                    break;
            }
        }

        cJSON_AddItemToObject(obj, "type", types);
        cJSON_AddItemToObject(obj, "params", params);
        cJSON_AddItemToArray(root, obj);
    }

    char* json_str = cJSON_Print(root);
    cJSON_Delete(root);

    FILE* fp = fopen(filepath, "w");
    if (!fp) {
        perror("Failed to open file for writing");
        free(json_str);
        return -1;
    }

    fputs(json_str, fp);
	fflush(fp);
    fclose(fp);
    free(json_str);
    return 0;
}


void free_param_items(ParamItem* items, int count)
{
    for (int i = 0; i < count; ++i) {
        free(items[i].name);
        free(items[i].scpi_cmd);
        for (int j = 0; j < items[i].param_count; ++j) {
            if (items[i].values[j].type == PARAM_STRING) {
                free(items[i].values[j].value.str);
            }
        }
        free(items[i].values);
    }
    free(items);
}

void print_param_items(const ParamItem* items, int count)
{
    for (int i = 0; i < count; ++i) {
        printf("%s [%s]:", items[i].name, items[i].scpi_cmd);
        for (int j = 0; j < items[i].param_count; ++j) {
            switch (items[i].values[j].type) {
                case PARAM_INT32:   printf(" %d", items[i].values[j].value.i32); break;
                case PARAM_UINT32:  printf(" %u", items[i].values[j].value.u32); break;
                case PARAM_INT64:   printf(" %lld", items[i].values[j].value.i64); break;
                case PARAM_UINT64:  printf(" %llu", items[i].values[j].value.u64); break;
                case PARAM_DOUBLE:  printf(" %.3f", items[i].values[j].value.d); break;
                case PARAM_STRING:  printf(" \"%s\"", items[i].values[j].value.str); break;
            }
        }
        printf("\n");
    }
}

void init_param()
{
    const char* path = NULL;

    switch (configs.default_preset) {
        case FACTORY: path = FACTORY_PATH; break;
        case FLASH1: path = FLASH1_PATH; break;
        case FLASH2: path = FLASH2_PATH; break;
        case FLASH3: path = FLASH3_PATH; break;
        case FLASH4: path = FLASH4_PATH; break;
        default: return;
    }


	current_path = path;

    if (get_param_items()) {
        free_param_items(get_param_items(), get_param_count());
    }

    int count = 0;
    ParamItem* items = load_params_from_json(path, &count);

	//print_param_items(get_param_items(), count);
	apply_param_items_by_scpi(get_param_items(), count - 12);//最后12个不配置

	printf("load cmd file: %s\r\n", current_path);
}

void init_default_param()
{
	reg_write_by_name("pll_enable", 1);
    init_awg_ctx();
}

#define MAX_PRESETS 8

int load_system_config(const char* filename, SystemConfig* configs, int max_count)
{
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp);

    char* buffer = malloc(filesize + 1);
    if (!buffer) {
        perror("malloc");
        fclose(fp);
        return -1;
    }

    fread(buffer, 1, filesize, fp);
    buffer[filesize] = '\0';
    fclose(fp);

    cJSON* root = cJSON_Parse(buffer);
    free(buffer);
    if (!root || !cJSON_IsArray(root)) {
        fprintf(stderr, "Invalid JSON format\n");
        cJSON_Delete(root);
        return -1;
    }

    int count = cJSON_GetArraySize(root);
	printf("SYSTEM PARAM COUNT: %d\r\n", count);
    if (count > max_count) count = max_count;

    for (int i = 0; i < count; ++i) {
        cJSON* item = cJSON_GetArrayItem(root, i);
        if (!item) continue;

        cJSON* def     = cJSON_GetObjectItem(item, "default_preset");
		cJSON* dhcp  = cJSON_GetObjectItem(item, "dhcpen");
        cJSON* ip      = cJSON_GetObjectItem(item, "ip");
        cJSON* mask    = cJSON_GetObjectItem(item, "mask");
        cJSON* gw      = cJSON_GetObjectItem(item, "gw");
        cJSON* mac     = cJSON_GetObjectItem(item, "mac");
        cJSON* vendors = cJSON_GetObjectItem(item, "vendors");
        cJSON* model   = cJSON_GetObjectItem(item, "model");
        cJSON* serial  = cJSON_GetObjectItem(item, "serial");
        cJSON* version = cJSON_GetObjectItem(item, "version");

        if (!cJSON_IsNumber(def) || !cJSON_IsString(ip) || !cJSON_IsString(mask) ||
            !cJSON_IsString(gw) || !cJSON_IsString(mac)) {
            fprintf(stderr, "Invalid base field in item %d\n", i);
            continue;
        }

        configs[i].default_preset = def->valueint;
		configs[i].dhcpen = dhcp->valueint;
        strncpy(configs[i].ip, ip->valuestring, IP_STR_LEN - 1);
        strncpy(configs[i].mask, mask->valuestring, IP_STR_LEN - 1);
        strncpy(configs[i].gw, gw->valuestring, IP_STR_LEN - 1);
        strncpy(configs[i].mac, mac->valuestring, MAC_STR_LEN - 1);

		//set_mac_address("eth0", mac->valuestring);

        if (cJSON_IsString(vendors))
            strncpy(configs[i].vendors, vendors->valuestring, STR_SMALL_LEN - 1);
        else
            configs[i].vendors[0] = '\0';

        if (cJSON_IsString(model)) {
            strncpy(configs[i].model, model->valuestring, STR_MEDIUM_LEN - 1);
			if(strncmp(configs[i].model, DEVICE_MODEL_STR_M005M_VV, strlen(DEVICE_MODEL_STR_M005M_VV)) == 0) {
				g_device_model = DEVICE_MODEL_M005M_VV;
				printf("Device Model: %s\r\n", DEVICE_MODEL_STR_M005M_VV);
			}else if(strncmp(configs[i].model, DEVICE_MODEL_STR_M005M_VI, strlen(DEVICE_MODEL_STR_M005M_VI)) == 0) {
				g_device_model = DEVICE_MODEL_M005M_VI;
				printf("Device Model: %s\r\n", DEVICE_MODEL_STR_M005M_VI);
			}else if(strncmp(configs[i].model, DEVICE_MODEL_STR_M005M, strlen(DEVICE_MODEL_STR_M005M)) == 0) {
				g_device_model = DEVICE_MODEL_M005M;
				printf("Device Model: %s\r\n", DEVICE_MODEL_STR_M005M);
			}else if(strncmp(configs[i].model, DEVICE_MODEL_STR_L500K, strlen(DEVICE_MODEL_STR_L500K)) == 0) {
				g_device_model = DEVICE_MODEL_L500K;
				printf("Device Model: %s\r\n", DEVICE_MODEL_STR_L500K);
			}else {
				printf("Unknow Device Model\r\n");
			}
        } else {
            configs[i].model[0] = '\0';
        }

        if (cJSON_IsString(serial))
            strncpy(configs[i].serial, serial->valuestring, STR_SMALL_LEN - 1);
        else
            configs[i].serial[0] = '\0';

        if (cJSON_IsString(version))
            strncpy(configs[i].version, version->valuestring, STR_SMALL_LEN - 1);
        else
            configs[i].version[0] = '\0';
    }

    cJSON_Delete(root);

#if 0
	if(configs[0].dhcpen) {
		start_dhcp_ip();
	}else {
		start_static_ip(configs[0].ip, configs[0].gw);
	}
#endif
	printf("load system file: %s\r\n", filename);
    return count;
}


int save_system_config(const char* filename, const SystemConfig* configs, int count)
{
    cJSON* root = cJSON_CreateArray();
    if (!root) return -1;

    for (int i = 0; i < count; ++i) {
        const SystemConfig* cfg = &configs[i];

        cJSON* item = cJSON_CreateObject();
        if (!item) {
            cJSON_Delete(root);
            return -1;
        }

        cJSON_AddNumberToObject(item, "default_preset", cfg->default_preset);
		cJSON_AddNumberToObject(item, "dhcpen", cfg->dhcpen);
        cJSON_AddStringToObject(item, "ip", cfg->ip);
        cJSON_AddStringToObject(item, "mask", cfg->mask);
        cJSON_AddStringToObject(item, "gw", cfg->gw);
        cJSON_AddStringToObject(item, "mac", cfg->mac);

		cJSON_AddStringToObject(item, "vendors", cfg->vendors);
		cJSON_AddStringToObject(item, "model", cfg->model);
		cJSON_AddStringToObject(item, "serial", cfg->serial);
		cJSON_AddStringToObject(item, "version", cfg->version);

        cJSON_AddItemToArray(root, item);
    }

    char* json_str = cJSON_Print(root);
    cJSON_Delete(root);

    if (!json_str) return -1;

    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        perror("fopen");
        free(json_str);
        return -1;
    }

    fwrite(json_str, 1, strlen(json_str), fp);
    fclose(fp);
    free(json_str);
    return 0;
}

int load_mapping_table(const char *filepath, MappingTable *table)
{
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        perror("open mapping file");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *content = malloc(size + 1);
    fread(content, 1, size, fp);
    content[size] = '\0';
    fclose(fp);

    cJSON *root = cJSON_Parse(content);
    free(content);
    if (!root) {
        fprintf(stderr, "parse JSON failed\n");
        return -1;
    }

    //const char *rangeStr = "1234567890123456789";

    //uint64_t rangeValue = strtoull("11566278764002000000", NULL, 10);
    //double xx = rangeValue;

	//printf("Converted value: %f\r\n", xx);
    //printf("Converted value: %llu\n", rangeValue);

    // voltage1
    cJSON *voltage1 = cJSON_GetObjectItem(root, "voltage1");
    table->voltage1_count = 0;
    if (cJSON_IsArray(voltage1)) {
        int n = cJSON_GetArraySize(voltage1);
        for (int i = 0; i < n && i < MAX_MAP_ITEMS; i++) {
            cJSON *item = cJSON_GetArrayItem(voltage1, i);
            table->voltage1[i].range = cJSON_GetObjectItem(item, "range")->valuedouble;
            table->voltage1[i].X  = strtoull(cJSON_GetObjectItem(item, "X")->valuestring, NULL, 10);
			table->voltage1[i].Y  = strtoull(cJSON_GetObjectItem(item, "Y")->valuestring, NULL, 10);
			table->voltage1[i].R  = strtoull(cJSON_GetObjectItem(item, "R")->valuestring, NULL, 10);
			table->voltage1[i].R1 = strtoull(cJSON_GetObjectItem(item, "R1")->valuestring, NULL, 10);
            table->voltage1_count++;
        }
    }

	// voltage2
    cJSON *voltage2 = cJSON_GetObjectItem(root, "voltage2");
    table->voltage2_count = 0;
    if (cJSON_IsArray(voltage2)) {
        int n = cJSON_GetArraySize(voltage2);
        for (int i = 0; i < n && i < MAX_MAP_ITEMS; i++) {
            cJSON *item = cJSON_GetArrayItem(voltage2, i);
            table->voltage2[i].range = cJSON_GetObjectItem(item, "range")->valuedouble;
            table->voltage2[i].X  = strtoull(cJSON_GetObjectItem(item, "X")->valuestring, NULL, 10);
			table->voltage2[i].Y  = strtoull(cJSON_GetObjectItem(item, "Y")->valuestring, NULL, 10);
			table->voltage2[i].R  = strtoull(cJSON_GetObjectItem(item, "R")->valuestring, NULL, 10);
			table->voltage2[i].R1 = strtoull(cJSON_GetObjectItem(item, "R1")->valuestring, NULL, 10);
            table->voltage2_count++;
        }
    }

    // current
    cJSON *current = cJSON_GetObjectItem(root, "current");
    table->current_count = 0;
    if (cJSON_IsArray(current)) {
        int n = cJSON_GetArraySize(current);
        for (int i = 0; i < n && i < MAX_MAP_ITEMS; i++) {
            cJSON *item = cJSON_GetArrayItem(current, i);
            table->current[i].range = cJSON_GetObjectItem(item, "range")->valuedouble;
            table->current[i].X  = strtoull(cJSON_GetObjectItem(item, "X")->valuestring, NULL, 10);
			table->current[i].Y  = strtoull(cJSON_GetObjectItem(item, "Y")->valuestring, NULL, 10);
			table->current[i].R  = strtoull(cJSON_GetObjectItem(item, "R")->valuestring, NULL, 10);
			table->current[i].R1 = strtoull(cJSON_GetObjectItem(item, "R1")->valuestring, NULL, 10);
            table->current_count++;
        }
    }

    cJSON_Delete(root);


	printf("Voltage1:\r\n");
	for (int i = 0; i < table->voltage1_count; ++i) {
		printf("  range=%.3f	X=%llu  Y=%llu	R=%llu	R1=%llu\n",
			   table->voltage1[i].range, table->voltage1[i].X, table->voltage1[i].Y, table->voltage1[i].R, table->voltage1[i].R1);
	}
	printf("Voltage2:\r\n");
	for (int i = 0; i < table->voltage2_count; ++i) {
		printf("  range=%.3f	X=%llu  Y=%llu	R=%llu	R1=%llu\n",
			   table->voltage2[i].range, table->voltage2[i].X, table->voltage2[i].Y, table->voltage2[i].R, table->voltage2[i].R1);
	}
	printf("Current:\r\n");
	for (int i = 0; i < table->current_count; ++i) {
		printf("  range=%.9f	X=%llu  Y=%llu R=%llu	R1=%llu\n",
			   table->current[i].range, table->current[i].X, table->current[i].Y, table->current[i].R, table->current[i].R1);
	}


    return 0;
}



int init_system_param()
{
	int count = load_system_config(SYSTEM_PATH, &configs, MAX_PRESETS);
	const char *mmap_path = MMAPTABLE_PATH;

    if (count <= 0) {
        printf("Failed to load or empty config\n");
        return -1;
    }

	if ((g_device_model == DEVICE_MODEL_M005M_VV) || (g_device_model == DEVICE_MODEL_M005M_VI)) {
		mmap_path = MMAPTABLE_VVVI_PATH;
	} else if (g_device_model == DEVICE_MODEL_M005M) {
		mmap_path = MMAPTABLE_M005M_PATH;
	} else if (g_device_model == DEVICE_MODEL_L500K) {
		mmap_path = MMAPTABLE_L500K_PATH;
	}

	if (load_mapping_table(mmap_path, &g_cvmap) != 0) {
        printf("open map file failed: %s\r\n", mmap_path);
        return -1;
    }

	return 0;
}


void send_file(int sock, struct sockaddr_in* client_addr, socklen_t addr_len, const char* filename, int id)
{
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        return;
    }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* json = malloc(filesize);
    if (!json) {
        perror("malloc");
        fclose(fp);
        return;
    }

    if (fread(json, 1, filesize, fp) != filesize) {
        perror("fread");
        free(json);
        fclose(fp);
        return;
    }
    fclose(fp);

    int total_packets = (filesize + PACKET_DATA_SIZE - 1) / PACKET_DATA_SIZE;

    for (int i = 0; i < total_packets; ++i) {
        UdpPacket pkt;
		pkt.file_id = id;
        pkt.seq_id = i;  // network byte order
        pkt.total  = total_packets;
        pkt.len    = (i == total_packets - 1) ? (filesize % PACKET_DATA_SIZE) : PACKET_DATA_SIZE;
        memcpy(pkt.data, json + i * PACKET_DATA_SIZE, pkt.len);

        size_t send_len = sizeof(pkt.file_id) + sizeof(pkt.seq_id) + sizeof(pkt.total) + sizeof(pkt.len) + pkt.len;

        sendto(sock, &pkt, send_len, 0, (struct sockaddr*)client_addr, addr_len);
        usleep(10000); // �򵥽���
    }

    printf("Sent '%s' as %d packets (%ld bytes)\n", filename, total_packets, filesize);
    free(json);
}


void *udp_cmd_thread(void *arg)
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr = {0}, client_addr = {0};
    socklen_t addr_len = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }

    printf("UDP server listening on port %d...\n", SERVER_PORT);

    while (1) {
        char cmd_buf[CMD_BUF_SIZE] = {0};
        int recv_len = recvfrom(sock, cmd_buf, sizeof(cmd_buf), 0,
                                (struct sockaddr*)&client_addr, &addr_len);
        if (recv_len <= 0) continue;

        printf("Received from %s:%d -> %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), cmd_buf);

        switch (cmd_buf[0]) {
            case '1':
                send_file(sock, &client_addr, addr_len, current_path, 1);
                break;
            case '2':
                send_file(sock, &client_addr, addr_len, SYSTEM_PATH, 2);
                break;
            default:
                printf("Unknown command: %c\n", cmd_buf[0]);
                break;
        }
    }

    close(sock);
    return 0;
}

/*****************************************************************************
 �� �� ��  : param_thread
 ��������  : ��λ�������������ù������ù���
 �������? : void *arg
 �������? : ��
 �� �� ֵ  : void *
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2025��11��18��
    ��    ��   : huangwei
    �޸�����   : �����ɺ���

*****************************************************************************/
void *param_thread(void *arg)
{
	ParamValue getpv,setpv;
	MappingTable *pmt = NULL;
	//[0]
	int cur_trig_src,last_trig_src;
	int cur_trig_srcrange,last_trig_srcrange;
	int trig_srcref;
	uint64_t min_uval, max_uval;
	int64_t min_val, max_val;
	double  trig_level, trig_hys;

	get_cv_maptable(&pmt);

	while(1) {
		//[0]����LEVEL<--����Դ<--����Դ��Χ
		get_param_by_name(PARAM_CURRENT,"trig_source", 0, &getpv);
		cur_trig_src = getpv.value.i32;
		get_param_by_name(PARAM_LAST,"trig_source", 0, &getpv);
		last_trig_src = getpv.value.i32;

		if(cur_trig_src == (TS_M005M_1_VOL - 1)) {
				get_param_by_name(PARAM_CURRENT,"si_volch1_range", 1, &getpv);
				cur_trig_srcrange = getpv.value.i32; //index
				get_param_by_name(PARAM_LAST,"si_volch1_range", 1, &getpv);
				last_trig_srcrange = getpv.value.i32;
		}else if(cur_trig_src == (TS_M005M_2_VOLCUR - 1)) {
			if(g_device_model == DEVICE_MODEL_M005M_VV) {
				get_param_by_name(PARAM_CURRENT,"si_volch2_range", 1, &getpv);
				cur_trig_srcrange = getpv.value.i32;
				get_param_by_name(PARAM_LAST,"si_volch2_range", 1, &getpv);
				last_trig_srcrange = getpv.value.i32;
				trig_srcref = pmt->voltage2[cur_trig_srcrange].range;
			}else if(g_device_model == DEVICE_MODEL_M005M_VI) {
				get_param_by_name(PARAM_CURRENT,"si_curch_range", 0, &getpv);
				cur_trig_srcrange = getpv.value.i32;
				get_param_by_name(PARAM_LAST,"si_curch_range", 0, &getpv);
				last_trig_srcrange = getpv.value.i32;
				trig_srcref = pmt->current[cur_trig_srcrange].range;
			}
		}else {

			continue;
		}


		if((cur_trig_src != last_trig_src) || (cur_trig_srcrange != last_trig_srcrange)) {
			get_param_by_name(PARAM_CURRENT,"trig_level", 0, &getpv);
			trig_level = getpv.value.d;
			get_param_by_name(PARAM_CURRENT,"trig_hysteresis", 0, &getpv);
			trig_hys = getpv.value.d;

			max_val = trig_level + trig_hys;
			min_val = trig_level - trig_hys;
			max_uval = (uint64_t)((pow(2,19) - 1) * max_val / trig_srcref); //LEVEL����
			min_uval = (uint64_t)((pow(2,19) - 1) * min_val / trig_srcref); //LEVEL����

			reg_write_by_name("hysteresis_range_high_l", max_uval & 0xFFFFFFFF);
			reg_write_by_name("hysteresis_range_high_h", (max_uval >> 32) & 0xFFFFFFFF);
			reg_write_by_name("hysteresis_range_low_l", max_uval & 0xFFFFFFFF);
			reg_write_by_name("hysteresis_range_low_h", (max_uval >> 32) & 0xFFFFFFFF);

			if(cur_trig_src == (TS_M005M_1_VOL - 1)) {
				get_param_by_name(PARAM_CURRENT,"si_volch1_range", 1, &getpv);
				set_param_by_name(PARAM_LAST,"si_volch1_range", 1, getpv);
			}else if(cur_trig_src == (TS_M005M_2_VOLCUR - 1)) {
				if(g_device_model == DEVICE_MODEL_M005M_VV) {
					get_param_by_name(PARAM_CURRENT,"si_volch2_range", 1, &getpv);
					set_param_by_name(PARAM_LAST,"si_volch2_range", 1, getpv);
				}else if(g_device_model == DEVICE_MODEL_M005M_VI) {
					get_param_by_name(PARAM_CURRENT,"si_curch_range", 0, &getpv);
					set_param_by_name(PARAM_LAST,"si_curch_range", 0, getpv);
				}
			}else {

			}
			get_param_by_name(PARAM_CURRENT,"trig_source", 0, &getpv);
			set_param_by_name(PARAM_LAST,"trig_source", 0, getpv);

			printf("trig src or src range is change\r\n");
		}


		usleep(100);
	}

}



