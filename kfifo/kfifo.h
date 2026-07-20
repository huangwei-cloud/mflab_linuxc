#ifndef _KFIFO_H_
#define _KFIFO_H_

#include <pthread.h>

struct __kfifo {

	unsigned int in;
	unsigned int out;
	unsigned int mask;
	unsigned int size;
	void		*data;

	pthread_mutex_t lock;  // ¼ÓÒ»¸öËø
};

extern int __kfifo_init(struct __kfifo *fifo, void *buffer, unsigned int size);

extern void __kfifo_reset(struct __kfifo *fifo);

extern int __kfifo_alloc(struct __kfifo *fifo, unsigned int size);

extern void __kfifo_free(struct __kfifo *fifo);

extern int __kfifo_in(struct __kfifo *fifo, const void *buf, unsigned int len);

extern int __kfifo_out(struct __kfifo *fifo, void *buf, unsigned int len);

int get_kfifo_validlen(struct __kfifo *fifo);

void get_kfifo_instance(struct __kfifo **fifo);

void kfifo_init();


#endif

