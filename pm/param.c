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
#include <inttypes.h>
#include "../ring/ringbuffer.h"
#include "../awg/awg.h"
#include "../clk/reg.h"
#include "../calib/soscalecalib_mfilab.h"
#include "../log/log.h"



int g_device_model = DEVICE_MODEL_UNKNOW;

static MappingTable g_cvmap;

static ParamItem* g_param_items = NULL;
static ParamItem* g_last_param_items = NULL;
static int g_param_count = 0;
static int g_param_sync_refresh = 0;

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
				printf("[%s][%d]:Local Param Type Error: name=%s, index=%d, old_type=%d, new_type=%d\r\n",
					__FUNCTION__, __LINE__, name, index, (int)old->type, (int)new_value.type);
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

    reapply_param();
	printf("load cmd file: %s\r\n", current_path);
}

void reapply_param(void)
{
    /* Reuse the live parameter table; other threads may still be reading it. */
    if (!get_param_items()) return;

    //init osc continue work
    reg_write_by_name("oscillators1_en", 1);
    reg_write_by_name("oscillators2_en", 1);
    reg_write_by_name("oscillators3_en", 1);
    reg_write_by_name("oscillators4_en", 1);
    reg_write_by_name("oscillators5_en", 1);
    reg_write_by_name("oscillators6_en", 1);
    reg_write_by_name("oscillators7_en", 1);
    reg_write_by_name("oscillators8_en", 1);

	apply_param_items_by_scpi(get_param_items(), get_param_count() - 11);
	/* Replay may leave dependency values unchanged; force their hardware writes once. */
	__atomic_store_n(&g_param_sync_refresh, 1, __ATOMIC_RELEASE);
}

void init_default_param()
{
    char name[64];
	int ch;

    // PID固定码值：Upper和Offset为2^63-1，Lower为-2^63，不做物理量换算。
	for(ch = 1; ch <= 4; ++ch) {
		sprintf(name, "pid%d_upper_limit_l", ch);
		reg_write_by_name(name, 0xFFFFFFFFU);
		sprintf(name, "pid%d_upper_limit_h", ch);
		reg_write_by_name(name, 0x7FFFFFFFU);
		sprintf(name, "pid%d_lower_limit_l", ch);
		reg_write_by_name(name, 0x00000000U);
		sprintf(name, "pid%d_lower_limit_h", ch);
		reg_write_by_name(name, 0x80000000U);
		sprintf(name, "pid%d_output_offset_l", ch);
		reg_write_by_name(name, 0);
		sprintf(name, "pid%d_output_offset_h", ch);
		reg_write_by_name(name, 0);
		// 64位码值写完后拉高、拉低锁存。
		sprintf(name, "pid%d_bigwidth_write_en", ch);
		reg_write_by_name(name, 1);
		reg_write_by_name(name, 0);
	}

	reg_write_by_name("pll_enable", 1);
    init_awg_ctx();//initial awg default

    //initial ad/da default delay;
    reg_write_by_name("ltc2000_1_odelay", 2);
    reg_write_by_name("ltc2000_2_odelay", 2);
    reg_write_by_name("ad9684_idelay", 100);
    reg_write_by_name("load_delay", 0);
    reg_write_by_name("load_delay", 1);
    reg_write_by_name("load_delay", 0);

    /* FPGA 识别 osc_work_all 的上升沿，写 0-1-0 产生一次同步脉冲。 */
	// reg_write_by_name("osc_work_all", 0);
	// reg_write_by_name("osc_work_all", 1);
	// reg_write_by_name("osc_work_all", 0);
    //others

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
			}else if(strncmp(configs[i].model, DEVICE_MODEL_STR_MFI_LAB, strlen(DEVICE_MODEL_STR_MFI_LAB)) == 0) {
				g_device_model = DEVICE_MODEL_MFI_LAB;
				printf("Device Model: %s\r\n", DEVICE_MODEL_STR_MFI_LAB);
			}else {
				printf("Unknown Device Model\r\n");
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

    return 0;
}
int init_system_param()
{
	int count = load_system_config(SYSTEM_PATH, &configs, MAX_PRESETS);
	const char *mmap_path = MMAPTABLE_MFILAB_PATH;

    if (count <= 0) {
        printf("Failed to load or empty config\n");
        return -1;
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
        usleep(10000);
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

typedef struct {
	const char *param_name;
	int index;
} param_dep_item_t;

typedef struct {
	const char *chain_name;
	int (*sync_cb)(int channel, int first_run);
	const param_dep_item_t items[8];
	int channel;
} param_dep_chain_t;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static int param_value_to_int(const ParamValue *pv, int *value)
{
	if(pv == NULL || value == NULL) {
		return -1;
	}

	if(pv->type == PARAM_INT32) {
		*value = pv->value.i32;
	} else if(pv->type == PARAM_UINT32) {
		*value = (int)pv->value.u32;
	} else if(pv->type == PARAM_DOUBLE) {
		*value = (int)pv->value.d;
	} else {
		return -1;
	}

	return 0;
}

static int param_value_to_double(const ParamValue *pv, double *value)
{
	if(pv == NULL || value == NULL) {
		return -1;
	}

	if(pv->type == PARAM_DOUBLE) {
		*value = pv->value.d;
	} else if(pv->type == PARAM_INT32) {
		*value = (double)pv->value.i32;
	} else if(pv->type == PARAM_UINT32) {
		*value = (double)pv->value.u32;
	} else {
		return -1;
	}

	return 0;
}

static int param_value_equal(const ParamValue *a, const ParamValue *b)
{
	if(a == NULL || b == NULL || a->type != b->type) {
		return 0;
	}

	switch(a->type) {
	case PARAM_INT32:
		return a->value.i32 == b->value.i32;
	case PARAM_UINT32:
		return a->value.u32 == b->value.u32;
	case PARAM_INT64:
		return a->value.i64 == b->value.i64;
	case PARAM_UINT64:
		return a->value.u64 == b->value.u64;
	case PARAM_DOUBLE:
		return a->value.d == b->value.d;
	case PARAM_STRING:
		return strcmp(a->value.str, b->value.str) == 0;
	default:
		return 0;
	}
}

static int param_dep_chain_changed(const param_dep_chain_t *chain)
{
	int i;
	ParamValue current;
	ParamValue last;

	for(i = 0; chain->items[i].param_name != NULL; i++) {
		if(get_param_by_name(PARAM_CURRENT, chain->items[i].param_name, chain->items[i].index, &current) != 0) {
			continue;
		}
		if(get_param_by_name(PARAM_LAST, chain->items[i].param_name, chain->items[i].index, &last) != 0) {
			return 1;
		}
		if(!param_value_equal(&current, &last)) {
			return 1;
		}
	}

	return 0;
}

static void param_dep_chain_update_last(const param_dep_chain_t *chain)
{
	int i;
	ParamValue current;

	for(i = 0; chain->items[i].param_name != NULL; i++) {
		if(get_param_by_name(PARAM_CURRENT, chain->items[i].param_name, chain->items[i].index, &current) == 0) {
			set_param_by_name(PARAM_LAST, chain->items[i].param_name, chain->items[i].index, current);
		}
	}
}

static int sync_signalout_xyr_scale(int channel, int first_run)
{
	(void)channel;
	(void)first_run;
	ParamValue pv;
	int ch;
	int source;
	int demo_source;
	int input_range;
	double user_scale;
	double output_scale;
	uint64_t scale_code;
	const char *source_param_name;
	const char *scale_param_name;
	const char *scale_reg_l_name;
	const char *scale_reg_h_name;
	const char *scale_reg_en_name;
	const char *input_range_param_name;

	if(get_param_by_name(PARAM_CURRENT, "demo_source_ch1", 1, &pv) != 0 ||
	   param_value_to_int(&pv, &demo_source) != 0) {
		return -1;
	}

	for(ch = 0; ch < 2; ch++) {
		if(ch == 0) {
			source_param_name = "so_source1_ch1";
			scale_param_name = "so_scaling1_ch1";
			scale_reg_l_name = "signalout1_scaling1_l";
			scale_reg_h_name = "signalout1_scaling1_h";
			scale_reg_en_name = "signalout1_bigwidth_write_en";
		} else {
			source_param_name = "so_source1_ch2";
			scale_param_name = "so_scaling1_ch2";
			scale_reg_l_name = "signalout2_scaling1_l";
			scale_reg_h_name = "signalout2_scaling1_h";
			scale_reg_en_name = "signalout2_bigwidth_write_en";
		}

		if(get_param_by_name(PARAM_CURRENT, source_param_name, 1, &pv) != 0 ||
		   param_value_to_int(&pv, &source) != 0) {
			continue;
		}

		if(get_param_by_name(PARAM_CURRENT, scale_param_name, 1, &pv) != 0 ||
		   param_value_to_double(&pv, &user_scale) != 0) {
			continue;
		}

		input_range_param_name = (demo_source == 0) ? "si_vol_range_ch1" : "si_vol_range_ch2";
		if(get_param_by_name(PARAM_CURRENT, input_range_param_name, 1, &pv) != 0 ||
		   param_value_to_int(&pv, &input_range) != 0) {
			continue;
		}

		if(source == 0 || source == 1 || source == 2) {
			/* SignalOut source:
			 *   0:X, 1:Y, 2:R
			 *
			 * For X/Y/R only:
			 *   +/-0.5 V input range: make it 1:1 at +/-2 V output, so scale = user_scale * 0.5 / 2.
			 *   +/-5 V input range: do not compensate; keep the original 5:2 output ratio.
			 */
			output_scale = (input_range == 0) ? (user_scale * 0.25) : user_scale;
		} else {
			output_scale = user_scale;
		}

		/* Start from the saved raw Scale each time; apply channel calibration once. */
		output_scale = soscale_mfilab_calibrate_value(ch + 1, output_scale);
		scale_code = (uint64_t)(output_scale * (double)((1ULL << 32) - 1));
		reg_write_by_name(scale_reg_l_name, (uint32_t)(scale_code & 0xFFFFFFFF));
		reg_write_by_name(scale_reg_h_name, (uint32_t)((scale_code >> 32) & 0xFFFFFFFF));
		reg_write_by_name(scale_reg_en_name, 1);
		reg_write_by_name(scale_reg_en_name, 0);

		P_LOG_DEBUG("param sync: so_ch%d source=%d demo_source=%d input_range=%d user_scale=%.9f output_scale=%.9f\r\n",
			ch + 1, source, demo_source, input_range, user_scale, output_scale);
	}

	return 0;
}

static int sync_pll_ref_hysteresis(int channel, int first_run)
{
	(void)channel;
	(void)first_run;
	ParamValue pv;
	int ref_source;
	int input_range;
	double range;
	double hys_high;
	double hys_low;
	int32_t high_code;
	int32_t low_code;
	const char *input_range_param_name;

	if(get_param_by_name(PARAM_CURRENT, "ref_source", 0, &pv) != 0 ||
	   param_value_to_int(&pv, &ref_source) != 0) {
		return -1;
	}

	/* PLL reference source:
	 *   ref_source 0:IN1, 1:IN2
	 *   si_vol_range 0:+/-0.5 V, 1:+/-5 V
	 */
	input_range_param_name = (ref_source == 0) ? "si_vol_range_ch1" : "si_vol_range_ch2";
	if(get_param_by_name(PARAM_CURRENT, input_range_param_name, 1, &pv) != 0 ||
	   param_value_to_int(&pv, &input_range) != 0) {
		return -1;
	}

	if(get_param_by_name(PARAM_CURRENT, "ref_hys_high", 0, &pv) != 0 ||
	   param_value_to_double(&pv, &hys_high) != 0) {
		return -1;
	}

	if(get_param_by_name(PARAM_CURRENT, "ref_hys_low", 0, &pv) != 0 ||
	   param_value_to_double(&pv, &hys_low) != 0) {
		return -1;
	}

	range = (input_range == 0) ? 0.5 : 5.0;
	high_code = (int32_t)(hys_high * (pow(2, 19) - 1) / range);
	low_code = (int32_t)(hys_low * (pow(2, 19) - 1) / range);

	reg_write_by_name("pll_ref_hysteresis_high", high_code);
	reg_write_by_name("pll_ref_hysteresis_low", low_code);

	P_LOG_DEBUG("param sync: pll_ref source=%d input_range=%d range=%.3f high=%.9f low=%.9f high_code=%d low_code=%d\r\n",
		ref_source, input_range, range, hys_high, hys_low, high_code, low_code);

	return 0;
}

static int sync_pid_r_setpoint(int ch, int first_run)
{
	ParamValue pv, last_value, last_source, last_demo, last_range;
	MappingTable *cvmt = NULL;
	PIDSource source;
	int demo_source, input_range;
	double range, value;
	int64_t setpoint;
	int ret_l, ret_h, ret_on, ret_off;
	char reg_name[64], param_name[64];

	get_cv_maptable(&cvmt);
	sprintf(param_name, "pid_source_ch%d", ch);
	get_param_by_name(PARAM_CURRENT, param_name, 1, &pv);
	source = (PIDSource)pv.value.d;
	sprintf(param_name, "pid_setpoint_ch%d", ch);
	get_param_by_name(PARAM_CURRENT, param_name, 1, &pv);
	value = pv.value.d;
	P_LOG_DEBUG("[PID_SETPOINT] enter: ch=%d, first_run=%d, source=0x%02x, target=%.12g, target_type=%d",
		ch, first_run, (unsigned)source, value, (int)pv.type);

	if(source == PID_SOURCE_DEMOD1_R) {
		/* Target (V) -> DEMOD1 input -> input range; reverse the Scope R formula. */
		get_param_by_name(PARAM_CURRENT, "demo_source_ch1", 1, &pv);
		demo_source = (int)pv.value.d; /* 0=IN1, 1=IN2. */
		if(demo_source == 0) {
			get_param_by_name(PARAM_CURRENT, "si_vol_range_ch1", 1, &pv);
			input_range = pv.value.i32;
			range = cvmt->voltage1[input_range].range;
		}else if(demo_source == 1) {
			get_param_by_name(PARAM_CURRENT, "si_vol_range_ch2", 1, &pv);
			input_range = pv.value.i32;
			range = cvmt->voltage2[input_range].range;
		}else {
			P_LOG_DEBUG("[PID_SETPOINT] skip ch%d: DEMOD1 input source=%d is reserved", ch, demo_source);
			return 0; /* Constant input conversion is reserved. */
		}
		setpoint = (int64_t)(value / range * ((1ULL << 63) - 1));
	}else if(source >= PID_SOURCE_AUXIN1 && source <= PID_SOURCE_AUXIN4) {
		P_LOG_DEBUG("[PID_SETPOINT] skip ch%d: AUXIN source=0x%02x conversion is reserved", ch, (unsigned)source);
		return 0; /* TODO: AUXIN1..4 conversion. */
	}else if(source >= PID_SOURCE_AUXOUT1 && source <= PID_SOURCE_AUXOUT4) {
		P_LOG_DEBUG("[PID_SETPOINT] skip ch%d: AUXOUT source=0x%02x conversion is reserved", ch, (unsigned)source);
		return 0; /* TODO: AUXOUT1..4 conversion. */
	}else if(source >= PID_SOURCE_DEMOD1_X && source <= PID_SOURCE_DEMOD6_THETA) {
		P_LOG_DEBUG("[PID_SETPOINT] skip ch%d: DEMOD source=0x%02x conversion is reserved", ch, (unsigned)source);
		return 0; /* TODO: remaining DEMOD1..6 X/Y/R/theta conversions. */
	}else if(source >= PID_SOURCE_PID_OUT1 && source <= PID_SOURCE_PID_OUT4) {
		P_LOG_DEBUG("[PID_SETPOINT] skip ch%d: PID output source=0x%02x conversion is reserved", ch, (unsigned)source);
		return 0; /* TODO: PID output conversion. */
	}else {
		P_LOG_DEBUG("[PID_SETPOINT] skip ch%d: unknown source=0x%02x", ch, (unsigned)source);
		return 0;
	}

	/* A change on the unused input range must not rewrite this PID. */
	if(!first_run) {
		sprintf(param_name, "pid_setpoint_ch%d", ch);
		get_param_by_name(PARAM_LAST, param_name, 1, &last_value);
		sprintf(param_name, "pid_source_ch%d", ch);
		get_param_by_name(PARAM_LAST, param_name, 1, &last_source);
		get_param_by_name(PARAM_LAST, "demo_source_ch1", 1, &last_demo);
		sprintf(param_name, "si_vol_range_ch%d", demo_source + 1);
		get_param_by_name(PARAM_LAST, param_name, 1, &last_range);
		if(value == last_value.value.d && source == (PIDSource)last_source.value.d &&
			demo_source == (int)last_demo.value.d && input_range == last_range.value.i32) {
			P_LOG_DEBUG("[PID_SETPOINT] skip ch%d: target and active dependencies unchanged", ch);
			return 0;
		}
	}

	P_LOG_DEBUG("[PID_SETPOINT] convert: ch=%d, input=IN%d, range_index=%d, range=%.12g V, target=%.12g V, code=%" PRId64 " (0x%016" PRIx64 ")",
		ch, demo_source + 1, input_range, range, value, setpoint, (uint64_t)setpoint);
	sprintf(reg_name, "pid%d_set_point_l", ch);
	ret_l = reg_write_by_name(reg_name, (uint32_t)setpoint);
	sprintf(reg_name, "pid%d_set_point_h", ch);
	ret_h = reg_write_by_name(reg_name, (uint32_t)((uint64_t)setpoint >> 32));
	sprintf(reg_name, "pid%d_bigwidth_write_en", ch);
	ret_on = reg_write_by_name(reg_name, 1);
	ret_off = reg_write_by_name(reg_name, 0);
	P_LOG_DEBUG("[PID_SETPOINT] write ch%d: low=0x%08x (ret=%d), high=0x%08x (ret=%d), commit=1->0 (ret=%d/%d)",
		ch, (unsigned)(uint32_t)setpoint, ret_l, (unsigned)(uint32_t)((uint64_t)setpoint >> 32), ret_h, ret_on, ret_off);
	return 0;
}

static const param_dep_chain_t g_param_dep_chains[] = {
	{
		"signalout_xyr_scale",
		sync_signalout_xyr_scale,
		{
			{ "si_vol_range_ch1", 1 },
			{ "si_vol_range_ch2", 1 },
			{ "demo_source_ch1",  1 },
			{ "so_source1_ch1",   1 },
			{ "so_source1_ch2",   1 },
			{ "so_scaling1_ch1",  1 },
			{ "so_scaling1_ch2",  1 },
			{ NULL, 0 },
		},
		0,
	},
	{
		"pll_ref_hysteresis",
		sync_pll_ref_hysteresis,
		{
			{ "ref_source",       0 },
			{ "si_vol_range_ch1", 1 },
			{ "si_vol_range_ch2", 1 },
			{ "ref_hys_high",     0 },
			{ "ref_hys_low",      0 },
			{ NULL, 0 },
		},
		0,
	},
	{
		"pid1_r_setpoint",
		sync_pid_r_setpoint,
		{
			{ "pid_setpoint_ch1", 1 },
			{ "pid_source_ch1",   1 },
			{ "demo_source_ch1",  1 },
			{ "si_vol_range_ch1", 1 },
			{ "si_vol_range_ch2", 1 },
			{ NULL, 0 },
		},
		1,
	},
	{
		"pid2_r_setpoint",
		sync_pid_r_setpoint,
		{
			{ "pid_setpoint_ch2", 1 },
			{ "pid_source_ch2",   1 },
			{ "demo_source_ch1",  1 },
			{ "si_vol_range_ch1", 1 },
			{ "si_vol_range_ch2", 1 },
			{ NULL, 0 },
		},
		2,
	},
	{
		"pid3_r_setpoint",
		sync_pid_r_setpoint,
		{
			{ "pid_setpoint_ch3", 1 },
			{ "pid_source_ch3",   1 },
			{ "demo_source_ch1",  1 },
			{ "si_vol_range_ch1", 1 },
			{ "si_vol_range_ch2", 1 },
			{ NULL, 0 },
		},
		3,
	},
	{
		"pid4_r_setpoint",
		sync_pid_r_setpoint,
		{
			{ "pid_setpoint_ch4", 1 },
			{ "pid_source_ch4",   1 },
			{ "demo_source_ch1",  1 },
			{ "si_vol_range_ch1", 1 },
			{ "si_vol_range_ch2", 1 },
			{ NULL, 0 },
		},
		4,
	},
};

void *param_sync_thread(void *arg)
{
	size_t i;
	int first_run = 1;
	int chain_need_sync[ARRAY_SIZE(g_param_dep_chains)] = {0};

	(void)arg;

	while(1) {
		if(__atomic_exchange_n(&g_param_sync_refresh, 0, __ATOMIC_ACQ_REL)) {
			first_run = 1;
		}
		for(i = 0; i < ARRAY_SIZE(g_param_dep_chains); i++) {
			chain_need_sync[i] = first_run || param_dep_chain_changed(&g_param_dep_chains[i]);
		}

		for(i = 0; i < ARRAY_SIZE(g_param_dep_chains); i++) {
			if(chain_need_sync[i] && g_param_dep_chains[i].sync_cb != NULL) {
				g_param_dep_chains[i].sync_cb(g_param_dep_chains[i].channel, first_run);
			}
		}

		for(i = 0; i < ARRAY_SIZE(g_param_dep_chains); i++) {
			if(chain_need_sync[i]) {
				param_dep_chain_update_last(&g_param_dep_chains[i]);
			}
		}

		first_run = 0;

		usleep(100000);
	}
}


