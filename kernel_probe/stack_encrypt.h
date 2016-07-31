#ifndef __ENCRYPT_H__
#define __ENCRYPT_H__
#include <linux/memclear_para.h>
#include <linux/slab.h>
#include "stack_buf.h"

int encrypt(unsigned long user_rsp, unsigned long user_rbp, sbuf_t *sbuf, para_list_t *plt);
int decrypt(void);


#endif
