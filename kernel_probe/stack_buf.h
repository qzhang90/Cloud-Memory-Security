#ifndef __STACK_BUF_H__
#define __STACK_BUF_H__

#include <linux/list.h>


typedef struct hole{
	char *start;
	unsigned long len;
	struct list_head list;
}hole_t;

typedef struct stack_buf{
	char *buf;
	char *start;	
	unsigned long len;
	struct list_head hole_list;


	unsigned long cur;
	unsigned long cap;
}sbuf_t;

sbuf_t *init_stack_buf(size_t size);
int copy_stack_buf(sbuf_t **sbuf, char *buf, unsigned long len);
int restore_stack_buf(sbuf_t *sbuf);
void print_stack_buf(sbuf_t *sbuf);
#endif
