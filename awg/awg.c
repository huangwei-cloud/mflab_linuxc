#include <stdio.h>
#include <string.h>
#include "awg.h"
#include "../axidma/libaxidma.h"

awg_ctx_t g_awg_ctx;

void init_awg_ctx()
{
	int i;

	g_awg_ctx.cfg[AWG_CH1].mode = 1;
	g_awg_ctx.cfg[AWG_CH2].mode = 1;

	for(i = 0; i < AWG_QUEUE_MAX_STEPS; i++) {
		g_awg_ctx.cfg[AWG_CH1].queue[i].repeat = 1;
		g_awg_ctx.cfg[AWG_CH1].queue[i].wait_us = 0;
		g_awg_ctx.cfg[AWG_CH1].queue[i].play_num = i;

		g_awg_ctx.cfg[AWG_CH2].queue[i].repeat = 1;
		g_awg_ctx.cfg[AWG_CH2].queue[i].wait_us = 0;
		g_awg_ctx.cfg[AWG_CH2].queue[i].play_num = i;
	}
	
}

void awg_data_send(uint32_t chnum, uint8_t *buff, uint32_t size)
{
	int rtn;
	dma_context_t *ctx = NULL;
	
	get_dma_context(&ctx);

	memcpy(ctx->channel[chnum].txbuff, buff, size);
	
	rtn = axidma_oneway_transfer(ctx->dev, ctx->channel[chnum].tx_channel, ctx->channel[chnum].txbuff, size, true);
	if(rtn !=0 ) {
		printf("awg write failed\r\n");
	}
}




