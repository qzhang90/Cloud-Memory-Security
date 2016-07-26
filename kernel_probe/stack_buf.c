#include <linux/slab.h>
#include "stack_buf.h"


sbuf_t *init_stack_buf(size_t size){

	sbuf_t *sbuf = kmalloc(size*sizeof(sbuf_t), GFP_KERNEL);
	
	if(sbuf == NULL){
		printk(KERN_ERR "stack buf initialization fails\n");
	}else{
		sbuf->cur = 0;
		sbuf->cap = size;
	}
	return sbuf;
}

int copy_stack_buf(sbuf_t **sbuf, char *buf, unsigned long len){
	unsigned long tmp_cap;
	sbuf_t *tmp_sbuf, *t1, *t2;
	int i;

	unsigned long cur = (*sbuf)->cur;
	unsigned long cap = (*sbuf)->cap;
	if(cur == cap){
		/*sbuf is full, realloc before insert*/
		tmp_cap = 2*cap;
		tmp_sbuf = kmalloc(tmp_cap*sizeof(sbuf_t), GFP_KERNEL);
	
		if(tmp_sbuf == NULL){
			printk(KERN_ERR "sbuf realloc fails, tmp_cap = %ld\n", tmp_cap);
			return -1;
		}	

		tmp_sbuf->cap = tmp_cap;
		tmp_sbuf->cur = cur;

		for(i = 0; i < cur; i++){
			t1 = *sbuf + i;
			t2 = tmp_sbuf + i;
			
			t2->buf = kmalloc(t1->len, GFP_KERNEL);
			memcpy(t2->buf, t1->buf, t1->len);

			kfree(t1->buf);
			t2->start = t1->start;
			t2->len = t1->len;
		}

		kfree(*sbuf);
		*sbuf = tmp_sbuf;
	}

	//Insert the new item
	tmp_sbuf = *sbuf + cur;
	tmp_sbuf->buf = kmalloc(len, GFP_KERNEL);
	memcpy(tmp_sbuf->buf, buf, len);

	tmp_sbuf->start = buf;
	tmp_sbuf->len =len;

	(*sbuf)->cur++;

	printk("stack buf insert cur = %ld\n", (*sbuf)->cur);
	return 0;
}

int restore_stack_buf(sbuf_t *sbuf){
	int i;
	sbuf_t *tmp;
	unsigned long cur = sbuf->cur;

	for(i = 0; i < cur; i++){
		tmp = sbuf + i;
		memcpy((char *)tmp->start, tmp->buf, tmp->len);
		sbuf->cur--;
	}

	return 0;
}

void print_stack_buf(sbuf_t *sbuf){
	int i;
	sbuf_t *tmp;
	
	for(i = 0; i < sbuf->cur; i++){
		tmp = sbuf + i;
		printk("start = %p, len = %p\n", (char *)tmp->start, (char *)tmp->len);
	}
}
