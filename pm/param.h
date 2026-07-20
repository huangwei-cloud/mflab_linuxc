#ifndef _PARAM_H_
#define _PARAM_H_

#include <stdio.h>
#include <stdint.h>

#define FACTORY_PATH "/home/root/app/param/factory.json"
#define FLASH1_PATH  "/home/root/app/param/flash1.json"
#define FLASH2_PATH  "/home/root/app/param/flash2.json"
#define FLASH3_PATH  "/home/root/app/param/flash3.json"
#define FLASH4_PATH  "/home/root/app/param/flash4.json"

#define SYSTEM_PATH  "/home/root/app/param/system.json"
#define MMAPTABLE_PATH "/home/root/app/param/cvmaptable.json"
#define MMAPTABLE_VVVI_PATH "/home/root/app/param/liam005mvvvi/cvmaptable.json"
#define MMAPTABLE_M005M_PATH "/home/root/app/param/liam005m/cvmaptable.json"
#define MMAPTABLE_L500K_PATH "/home/root/app/param/lial500k/cvmaptable.json"

#define DEVICE_MODEL_STR_M005M_VV "LIA-M005M-VV" //ZYNQMPSOC
#define DEVICE_MODEL_STR_M005M_VI "LIA-M005M-VI" //ZYNQMPSOC
#define DEVICE_MODEL_STR_M005M    "LIA-M005M"    //ZYNQMPSOC
#define DEVICE_MODEL_STR_L500K    "LIA-L500K"    //ZYNQ7Z100
#define DEVICE_MODEL_STR_H050M    "LIA-H050M"
#define DEVICE_MODEL_STR_U600M    "LIA-U600M"
#define DEVICE_MODEL_STR_MFI_LAB  "MFI-LAB"


#define DEVICE_MODEL_UNKNOW   (0)
#define DEVICE_MODEL_M005M_VV (1)
#define DEVICE_MODEL_M005M_VI (2)
#define DEVICE_MODEL_M005M    (3)
#define DEVICE_MODEL_L500K 	  (4)
#define DEVICE_MODEL_H050M    (5)
#define DEVICE_MODEL_U600M    (6)
#define DEVICE_MODEL_MFI_LAB  (7)

#define DEVICE_DEMOD_NUM_UNKNOWN (0)
#define DEVICE_DEMOD_NUM_M005M   (6)
#define DEVICE_DEMOD_NUM_M005M_VV DEVICE_DEMOD_NUM_M005M
#define DEVICE_DEMOD_NUM_M005M_VI DEVICE_DEMOD_NUM_M005M
#define DEVICE_DEMOD_NUM_L500K   (8)
#define DEVICE_DEMOD_NUM_H050M   DEVICE_DEMOD_NUM_UNKNOWN
#define DEVICE_DEMOD_NUM_U600M   DEVICE_DEMOD_NUM_UNKNOWN
#define DEVICE_DEMOD_NUM_MFI_LAB DEVICE_DEMOD_NUM_UNKNOWN

#define PARAM_LAST 	  (1)
#define PARAM_CURRENT (2)

typedef enum _PARAM_FILE{
	FACTORY,
	FLASH1,
	FLASH2,
	FLASH3,
	FLASH4,
}ParamFile;

typedef enum {
    PARAM_INT32 = 0,
    PARAM_UINT32,
    PARAM_INT64,
    PARAM_UINT64,
    PARAM_DOUBLE,
    PARAM_STRING
} ParamType;

typedef struct {
    ParamType type;
    union {
        int i32;
        unsigned int u32;
        long long i64;
        unsigned long long u64;
        double d;
        char* str;
    } value;
} ParamValue;

#define MAX_PARAM_ITEMS 500

typedef struct {
    char* name;
    char* scpi_cmd;
    ParamValue* values;
    int param_count;
} ParamItem;

#define IP_STR_LEN 16
#define MAC_STR_LEN 18
#define STR_SMALL_LEN  32
#define STR_MEDIUM_LEN 64

typedef struct {
    int default_preset;
	int dhcpen;
    char ip[IP_STR_LEN];
    char mask[IP_STR_LEN];
    char gw[IP_STR_LEN];
    char mac[MAC_STR_LEN];

	char vendors[STR_SMALL_LEN];
    char model[STR_MEDIUM_LEN];
    char serial[STR_SMALL_LEN];
    char version[STR_SMALL_LEN];
} SystemConfig;


#define SERVER_PORT 9000
#define PACKET_DATA_SIZE 1000
#define CMD_BUF_SIZE 16

typedef struct {
	uint8_t  file_id;
    uint16_t seq_id;
    uint16_t total;
    uint16_t len;
    char data[PACKET_DATA_SIZE];
} __attribute__((packed)) UdpPacket;


#define MAX_MAP_ITEMS 16

typedef struct {
    double range;
    uint64_t X;
    uint64_t Y;
    uint64_t R;
    uint64_t R1;
} MapItem;

typedef struct {
    MapItem voltage1[MAX_MAP_ITEMS];
    int voltage1_count;
	MapItem voltage2[MAX_MAP_ITEMS];
    int voltage2_count;
    MapItem current[MAX_MAP_ITEMS];
    int current_count;
} MappingTable;

int get_device_model(void);
ParamItem* get_param_items(void);
int get_param_count(void);
void get_system_param(SystemConfig **sc);
void get_cv_maptable(MappingTable **cvmt);
int set_param_by_name(int objectindex, const char* name, int index, ParamValue new_value);
int get_param_by_name(int objectindex, const char* name, int index, ParamValue* out_value);

ParamItem* load_params_from_json(const char* filepath, int* out_count);
int save_params_to_json(const char* filepath, const ParamItem* items, int count);
void free_param_items(ParamItem* items, int count);
void print_param_items(const ParamItem* items, int count);

void init_param();
void init_default_param();

int load_system_config(const char* filename, SystemConfig* configs, int max_count);
int save_system_config(const char* filename, const SystemConfig* configs, int count);

int init_system_param();

void *udp_cmd_thread(void *arg);
void *param_thread(void *arg);

#endif // PM_H




















