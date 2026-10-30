#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include "main.h"
#include "application/app.h"
#include "kfifo/kfifo.h"
#include "utils/utils.h"
#include "utils/scpi-def.h"
#include "ring/ringbuffer.h"


extern int g_restart_config;/*default param config*/


int main(int argc, char *argv[])
{
	init();
	while(1) {
		if(g_restart_config)
		{
			init_param();
			g_restart_config = 0;
		}	
		sleep(10);
	}
	
	return 0;
}



