#ifndef __ENCRYPT_H__
#define __ENCRYPT_H__
#include <linux/memclear_para.h>
#include <linux/slab.h>
#include "stack_buf.h"

int encrypt_stack(unsigned long user_rsp, unsigned long user_rbp, para_list_t *plt, uint64_t key);
int decrypt_stack(unsigned long user_rsp, unsigned long user_rbp, para_list_t *plti, uint64_t key);

#endif
