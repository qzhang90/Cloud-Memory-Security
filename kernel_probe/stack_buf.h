#ifndef __STACK_BUF_H__
#define __STACK_BUF_H__

typedef struct stack_buf{
	char *buf;
	unsigned char *start;	
	unsigned long len;

	unsigned long cur;
	unsigned long cap;
}sbuf_t;

sbuf_t *init_stack_buf(size_t size);
int copy_stack_buf(sbuf_t **sbuf, char *buf, unsigned long len);
int restore_stack_buf(sbuf_t *sbuf);
void print_stack_buf(sbuf_t *sbuf);
#endif
