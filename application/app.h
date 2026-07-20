#ifndef _APP_H_
#define _APP_H_
#include <stdint.h>
#include "../clk/hmc7043.h"

/************UDP*************/
#define UDP_PACKET_SIZE (1024)

#pragma pack(push, 1)

typedef struct _UDP_PKT {
    uint32_t fid;
    uint16_t total;
    uint16_t index;
    uint8_t  payload[UDP_PACKET_SIZE];
}UdpPkt;

#pragma pack(pop)
/***************************/

/*************TCP***********/
#define TCP_PORT        50000
#define MAX_CLIENTS     16

#pragma pack(push, 1)

typedef struct _TCP_HDR {
    uint32_t magic;      // 固定 0xABCD1234 校验
    uint16_t ch;         // 通道号
    uint16_t reserved;   // 对齐用
    uint32_t frame_id;   // 帧号
    uint32_t payload_len;// 本帧字节数
} TcpHdr;

#pragma pack(pop)


/***************************/
void dev_clk_switch(RefClk rc);

void init();

void test_modules();

#endif




