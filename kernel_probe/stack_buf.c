#include <linux/slab.h>
#include <linux/list_sort.h>
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
			t2->hole_list = t1->hole_list;
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
	INIT_LIST_HEAD(&tmp_sbuf->hole_list);

	(*sbuf)->cur++;

	return 0;
}

static int cmp(void *priv, struct list_head *a, struct list_head *b){
	hole_t *ha, *hb;
	
	ha = container_of(a, hole_t, list);
	hb = container_of(b, hole_t, list);
	
	return (int)(ha->start - hb->start);
}

int restore_stack_buf(sbuf_t *sbuf){
	int i;
	sbuf_t *tmp;
	unsigned long cur = sbuf->cur, len;
	struct list_head *pos;
	hole_t *h;
	char *from, *to;

	for(i = 0; i < cur; i++){
		tmp = sbuf + i;
		
		from = tmp->buf;
		to = (char *)tmp->start;

		list_sort(NULL, &tmp->hole_list, cmp);

		list_for_each(pos, &tmp->hole_list){
                        h = list_entry(pos, hole_t, list);
			
			len = (unsigned long)(h->start - from);
			//printk("haha1 to = %p, from = %p, len =%ld\n", to, from, len);
			if(len != 0){
				memcpy(to, from, len);
			}
			from = h->start + h->len;
			to += len + h->len;
                }

		len = (unsigned long)(tmp->buf + tmp->len - from);
		//printk("haha2 to = %p, from = %p, len =%ld\n", to, from, len);
		memcpy(to, from, len);
		
		sbuf->cur--;
	}

	return 0;
}

void print_stack_buf(sbuf_t *sbuf){
	int i;
	sbuf_t *tmp;
	
	for(i = 0; i < sbuf->cur; i++){
		tmp = sbuf + i;
	}
}
