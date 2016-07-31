#include <linux/types.h>
#include "stack_encrypt.h"

int encrypt_stack(unsigned long user_rsp, unsigned long user_rbp, para_list_t *plt, uint64_t key){	
	uint64_t *p;
	unsigned long i, j, delta, total_para_size, offset; 
	para_t *tmp;
	char *buf;/*store parameter bytes*/

	key = 0x1111111111111111; /*should be a random number*/		
	/*copy parameters into buf*/
	total_para_size = 0;
	for(i = 0; i < plt->size; i++){
		tmp = plt->plist + i;
		total_para_size += tmp->size;	
	}
	buf = kmalloc(total_para_size, GFP_KERNEL);
	
	offset = 0;	
	for(i = 0; i < plt->size; i++){
		tmp = plt->plist + i;
		memcpy(buf + offset, (char *)tmp->addr, tmp->size);
		offset += tmp->size;
	}
	
	/*encrypt the whole stack*/
	for(p = (uint64_t *)user_rsp; p <= (uint64_t *)user_rbp; p++){
		*p = *p^key;
	}

	if((char *)p < (char *)user_rbp){
		delta = (char *)user_rbp - (char *)p;
		for(j = 0; j < delta; j++){
			*((char *)p + j) = *((char *)p + j)^(key&0xFF);	
		}
	}
	
	/*restore parameters*/	
	offset = 0;
	for(i = 0; i < plt->size; i++){
		tmp = plt->plist + i;
		memcpy((char *)tmp->addr, buf + offset, tmp->size);
		offset += tmp->size;
	}

	kfree(buf);
	return 0;
}

int decrypt_stack(unsigned long user_rsp, unsigned long user_rbp, para_list_t *plt, uint64_t key){
	uint64_t *p;
	unsigned long i, j, delta, offset, total_para_size;
	para_t *tmp;
	char *buf;

	key = 0x1111111111111111; /*should be a random number*/	

	/*copy parameters into buf*/
        total_para_size = 0;
        for(i = 0; i < plt->size; i++){
                tmp = plt->plist + i;
                total_para_size += tmp->size;
        }
        buf = kmalloc(total_para_size, GFP_KERNEL);

        offset = 0;
        for(i = 0; i < plt->size; i++){
                tmp = plt->plist + i;
                memcpy(buf + offset, (char *)tmp->addr, tmp->size);
                offset += tmp->size;
        }

	
	/*decrypt the whole stack*/
	for(p = (uint64_t *)user_rsp; p <= (uint64_t *)user_rbp; p++){
                *p = *p^key;
        }

        if((char *)p < (char *)user_rbp){
                delta = (char *)user_rbp - (char *)p;
                for(j = 0; j < delta; j++){
                        *((char *)p + j) = *((char *)p + j)^(key&0xFF);
                }
        }

	/*restore parameters*/
        offset = 0;
        for(i = 0; i < plt->size; i++){
                tmp = plt->plist + i;
                memcpy((char *)tmp->addr, buf + offset, tmp->size);
                offset += tmp->size;
        }

	kfree(buf);
	return 0;
}
