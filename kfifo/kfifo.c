#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "kfifo.h"

#define DEBUG
#define DEBUG_ERR

#ifdef DEBUG_ERR
#define debug_printf_err   printf
#else
#define debug_printf_err
#endif

#ifdef DEBUG
#define debug_printf  printf
#else
#define debug_printf
#endif

#define kfifo_free(x) do { if (x) free(x); } while (0)
#define kfifo_malloc(x)          malloc(x)
#define kfifo_memcpy(x, y ,size) memcpy(x, y, size)
#define kfifo_memset(x, y, size) memset(x, y, size)


/*判断x是否为2的幂*/
#define is_power_of_2(x)  ((x) != 0 && (((x) & ((x) - 1)) == 0))
/*取x和y中最小值*/
#define min(x,y)          (((x) < (y)) ? (x) : (y))

#define KFIFO_SIZE (256 * 1024)
struct __kfifo g_kfifo;
char kfifo_buffer[KFIFO_SIZE];


static inline unsigned int kfifo_unused(struct __kfifo *fifo)
{
	return (fifo->mask+1) - (fifo->in - fifo->out);
}


int __kfifo_init(struct __kfifo *fifo, void *buffer, unsigned int size)
{

	if(!is_power_of_2(size))
	{
		debug_printf_err("size is not 2^\n");
		return -1;
	}
	fifo->in = 0;
	fifo->out = 0;
	fifo->size = size;
	fifo->mask = size-1;

	fifo->data = buffer;

	pthread_mutex_init(&fifo->lock, NULL);  // 初始化锁

	return 0;

}
void __kfifo_reset(struct __kfifo *fifo)
{
	fifo->in = 0;
	fifo->out = 0;
	kfifo_memset(fifo->data, 0x00, fifo->size);
}


int __kfifo_alloc(struct __kfifo *fifo, unsigned int size)
{
	if(!is_power_of_2(size))
	{
		debug_printf_err("malloc fail ,size is not 2^\n");
		return -1;
	}

	fifo->in = 0;
	fifo->out = 0;

	fifo->data = malloc(size);

	if(!fifo->data)
	{
		fifo->mask = 0;
		return -1;
	}
	fifo->size = size;
	fifo->mask = size -1;

	pthread_mutex_init(&fifo->lock, NULL); // 初始化锁

	return 0;
}

void __kfifo_free(struct __kfifo *fifo)
{
	kfifo_free(fifo->data);

	fifo->in = 0;
	fifo->out = 0;

	fifo->mask = 0;
	fifo->size = 0;

	pthread_mutex_destroy(&fifo->lock);  // 销毁锁
}

static void kfifo_copy_in(struct __kfifo *fifo, const void *buf, unsigned int len, unsigned int offset)
{
	unsigned int size = fifo->mask + 1;
	unsigned int l;
	
	offset &=fifo->mask;
	l = min(len, size - offset);

	kfifo_memcpy((unsigned char *)fifo->data + offset, buf, l);
	kfifo_memcpy(fifo->data, (unsigned char *)buf + l, len - l);
	
}

static void kfifo_copy_out(struct __kfifo *fifo, void *buf, unsigned int len, unsigned int offset)
{
	unsigned int size = fifo->mask + 1;
	unsigned int l;
	 
	offset &= fifo->mask;
	l = min(len, size - offset);

	kfifo_memcpy(buf, (unsigned char *)fifo->data + offset, l);
	kfifo_memcpy((unsigned char *)buf + l, fifo->data, len - l);
}

int __kfifo_in(struct __kfifo *fifo, const void *buf, unsigned int len)
{
	unsigned int l;

	pthread_mutex_lock(&fifo->lock); // 加锁

	l = kfifo_unused(fifo);
	if(len > l)
		len = l;

	kfifo_copy_in(fifo, buf, len, fifo->in);
	fifo->in += len;

	pthread_mutex_unlock(&fifo->lock); // 解锁

	return len;
}

int __kfifo_out(struct __kfifo *fifo, void *buf, unsigned int len)
{
	unsigned int l;

	pthread_mutex_lock(&fifo->lock); // 加锁
	
	l = fifo->in - fifo->out;
	if(len > l)
		len = l;
	
	kfifo_copy_out(fifo, buf, len, fifo->out);
	fifo->out += len;

	pthread_mutex_unlock(&fifo->lock); // 解锁

	return len;
}

/* BEGIN: Modified by huangwei, 2025/3/19 */
int get_kfifo_validlen(struct __kfifo *fifo)
{
	int len;
	
	pthread_mutex_lock(&fifo->lock);
	len = fifo->in - fifo->out;
	pthread_mutex_unlock(&fifo->lock);
	return len;
}

void get_kfifo_instance(struct __kfifo **fifo)
{
	assert(fifo != NULL);

	*fifo = &g_kfifo;
}

void kfifo_init()
{
	__kfifo_init(&g_kfifo, kfifo_buffer, KFIFO_SIZE);
}

/* END:  Modified by huangwei, 2025/3/19 */

