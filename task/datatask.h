#ifndef _DATATASK_H_
#define _DATATASK_H_
#include "../utils/zwtypes.h"
#include "../ring/ringbuffer.h"

void *data_server_thread(void *arg);
int read_demo_rdata(DemoDaqData *prdata);
double load_double(const void *p, int channel);



#endif

