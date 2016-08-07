#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/aio.h>
#include <linux/string.h>
#include <linux/highmem.h>
#include <asm/kmap_types.h>
#include <linux/user.h>
#include <linux/regset.h>
#include <linux/slab.h>
#include <asm/user_64.h>
#include <asm/compat.h>
#include <linux/percpu.h>
#include <asm/processor.h>

#include "parse_elf.h"
#include "mapper.h"
#include "stack_buf.h"
#include <linux/memclear_para.h>

char *path_to_exe = "/home/test/ftest";

//This data structure is copied from arch/x86/kernel/ptrace.c
enum x86_regset {
	REGSET_GENERAL,
	REGSET_FP,
	REGSET_XFP,
	REGSET_IOPERM64 = REGSET_XFP,
	REGSET_XSTATE,
	REGSET_TLS,
	REGSET_IOPERM32,
};

#define ENCRYPT 0
#define DECRYPT 1

typedef struct call_stack_item{
	int op, key;
	unsigned long user_rsp, user_rbp;
	char *func_name;
}cs_item_t;

static int call_stack_top = 0;
cs_item_t* call_stack[64]; /*maximum depth of function calls*/


pid_t pid = -1;
struct page *p = NULL;
item_t *mapper;

static int sbuf_stack_top = 0;
sbuf_t *sbuf_stack[16]; /*each element of the stack pointing to a list of stack_bufs, each stack_buf relates to a function*/

static int init = 0;


/*
*Return how may bytes from @addr should not be encrypted, othersie 0
*/
unsigned long contains(unsigned long addr, para_list_t *plt){
	int i;
	para_t *tmp;

	for(i = 0; i < plt->size; i++){
		tmp = plt->plist + i;
		
		if(addr == tmp->addr){
			if(tmp->pointer != 0){
				return 0;
			}else{
				return tmp->size;
			}
		}
	}
	
	return 0;
}

char *find_in_stack_buf(char *addr, unsigned long len, sbuf_t *sbuf){
	int i;
	sbuf_t *tmp;

	for(i = 0; i < sbuf->cur; i++){
		tmp = sbuf + i;
		if((addr >= tmp->start) && (addr + len <= tmp->start + tmp->len)){
			return tmp->buf + (addr - tmp->start);/*find it, return the 'copy from' address*/
		}
	}
	return NULL; /*not found*/
}

int restore_pointers(para_list_t *plt, sbuf_t *sbuf){

	int i;
	para_t *tmp;
	unsigned long size;
	unsigned long org_value, addr_in_stack, addr_in_buf;
	hole_t *h;

	for(i = 0; i < plt->size; i++){
		tmp = plt->plist + i;
		
		if(tmp->pointer == 0){
			continue;		
		}else{
			addr_in_stack = tmp->addr;
			size = tmp->size;

			tmp->pointer--;

			addr_in_buf = (unsigned long)find_in_stack_buf((char *)addr_in_stack, sizeof(void *), sbuf);
			
			if(!addr_in_buf){
				printk(KERN_ERR "ERROR: add_in_buf is NULL");
				return -1;
			}

			org_value = *(unsigned long *)addr_in_buf;
			*(unsigned long *)addr_in_stack = org_value;

			h = kmalloc(sizeof(hole_t), GFP_KERNEL);
			h->start = (char *)addr_in_buf;
			h->len = sizeof(void *);
			list_add_tail(&h->list, &sbuf->hole_list);

			addr_in_stack = *(unsigned long *)addr_in_stack;

			while(tmp->pointer != 0){
				addr_in_buf = (unsigned long)find_in_stack_buf((char *)addr_in_stack, sizeof(void *), sbuf);
				
				org_value = *(unsigned long *)addr_in_buf;
				*(unsigned long *)addr_in_stack = org_value;
			
				h = kmalloc(sizeof(hole_t), GFP_KERNEL);
				h->start = (char *)addr_in_buf;
				h->len = sizeof(void *);
				list_add_tail(&h->list, &sbuf->hole_list);
				
				tmp->pointer--;
				addr_in_stack = *(unsigned long *)addr_in_stack;
			}

			addr_in_buf = (unsigned long)find_in_stack_buf((char *)addr_in_stack, size, sbuf);
			
			if(addr_in_buf){
				memcpy((char *)addr_in_stack, (char *)addr_in_buf, size);
				
				h = kmalloc(sizeof(hole_t), GFP_KERNEL);
				h->start = (char *)addr_in_buf;
				h->len = size;
				list_add_tail(&h->list, &sbuf->hole_list);
			}else{
				printk(KERN_ERR "Cannot find %p in stack bufs\n", p);
			}
		}
	}
	return 0;
}

static long jvfs_write(struct file *file, const char __user *buf, size_t count, loff_t *pos){
        char *file_name = file->f_path.dentry->d_iname;

	
        if(!strcmp(file_name, "text.txt")){  
	      	printk(KERN_INFO "jprobe: jvfs_write, %s, file->f_op->write = %pF\n", file_name, file->f_op->write);
		
		if(p){	
			char *kaddr = kmap_atomic(p);
			int i;
			printk(KERN_INFO "jvfs_write, previous page = %p, kaddr = %p, pid = %d, pfn = %ld\n", p, kaddr, pid, page_to_pfn(p));
			for(i = 0; i < 512; i++){
				printk("%02x ", (*(kaddr + i))&0xFF);
			}
			kunmap_atomic(kaddr);
		}
	}
	jprobe_return();
	return 0;
} 

static long jdo_sync_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos){
	printk(KERN_INFO "jprobe: jdo_sync_write, %pf\n", filp->f_op->aio_write);
	jprobe_return();
	return 0;
}

static long jgeneric_file_aio_write(struct kiocb *iocb, const struct iovec *iov,
		unsigned long nr_segs, loff_t pos)
{
        char *file_name = iocb->ki_filp->f_path.dentry->d_iname;
        if(!strcmp(file_name, "text.txt"))  
		printk(KERN_INFO "jprobe: generic_file_aio_write\n");
	jprobe_return();
	return 0;
}

static long jgeneric_file_buffered_write(struct kiocb *iocb, const struct iovec *iov,
		unsigned long nr_segs, loff_t pos, loff_t *ppos,
		size_t count, ssize_t written)
{
	char *file_name = iocb->ki_filp->f_path.dentry->d_iname;

	//if(!strcmp(file_name, "syslog") && !strcmp(file_name, "kern.log"))
        if(!strcmp(file_name, "text.txt")){
		printk(KERN_INFO "jprobe: generic_file_buffered_write,%s, len = %d, pid = %d\n", 
			file_name, (int)iov->iov_len, (int)current->pid);
	}
	jprobe_return();
	return 0;
}


static ssize_t jgeneric_perform_write(struct file *file,
				struct iov_iter *i, loff_t pos){

	char *file_name = file->f_path.dentry->d_iname;
	
	
        if(!strcmp(file_name, "text.txt")){  
		pid = current->pid;		
		printk(KERN_INFO "jprobe: generic_perform_write, %s, write_begin = %pF, pid = %d\n", 
			file_name, file->f_mapping->a_ops->write_begin, current->pid);
	}
	jprobe_return();
	return 0;
}

size_t jiov_iter_copy_from_user_atomic(struct page *page,
		struct iov_iter *i, unsigned long offset, size_t bytes){
	if(current->pid == pid){
		printk(KERN_INFO "jiov_iter_copy_from_user_atomic, bytes = %zu, pid = %d\n", bytes, current->pid);
		if(bytes == 512){
			p = page;
			printk(KERN_INFO "p = %p, pfn = %ld\n", p, page_to_pfn(p));
		}
	}
	jprobe_return();
	return 0;
}

void jmark_page_accessed(struct page *page){
	jprobe_return();
}

long jsys_encrypt_stack(void *arg){
	
	unsigned long i, skip;
	unsigned long prev_rsp;
	int len;
	cs_item_t *csit, *prev_csit;
	char *func_name;
	register unsigned long kernel_rbp asm("rbp");
	unsigned long user_ip = *((unsigned long *)kernel_rbp + 12);	
	unsigned long user_rbp = *(unsigned long *)kernel_rbp, prev_user_rbp;
	unsigned long user_rsp, prev_user_rsp;
	para_list_t *para_list = (para_list_t *)arg;
	const struct pt_regs *regs = task_pt_regs(current);
	sbuf_t *stack_buf;

	user_rsp = (unsigned long)((const void __user *)regs->sp + 0x750);
	
	/*
	for(i = 0; i < 0x750; i++){
		printk("%02x ", *((char *)user_rsp - i)&0XFF);
	}
	printk("\n\n\n\n\n");
	*/
	/*
	for(i = 0; i < para_list->size; i++){
		para = para_list->plist + i;
		printk("para%d: value =%d, size = %d, addr = %p\n", i, *(int *)para->addr, para->size, para->addr);	
	}	
	*/

	if(init == 0){
		mapper = init_mapper(8);

		for(i = 0; i < 16; i++){
			 sbuf_stack[i] = init_stack_buf(32);
		}
		parse(path_to_exe, &mapper);
		
		init = 1;
	}

	func_name = get_func_name(mapper, user_ip); 
	if(func_name == NULL){
		printk("%s: NULL func_name error\n", __func__);
		return 0;
	}else{
		//printk("user_rsp = %lx, user_ip = %ld, func_name = %s\n\n", user_rsp, user_ip, func_name);
	}

	/*check*/
	if(user_rbp < user_rsp){
		printk(KERN_ERR "ERROR: Illegal stack encrypt from function %s, rbp should not be smaller than rsp\n"
			, func_name);
		printk(KERN_ERR "ERROR: user_rbp = %p, user_rsp = %p\n"
			,(char *)user_rbp, (char *)user_rsp);
		return -1;
	}

	/*check*/
	if(call_stack_top > 0){
		prev_csit = call_stack[call_stack_top - 1];
		printk("get prev_csit from call stack # %d\n", call_stack_top - 1);
		prev_user_rbp = prev_csit->user_rbp;
		prev_user_rsp = prev_csit->user_rsp;
	
		if(user_rbp >= prev_user_rsp){
			printk(KERN_ERR "ERROR: Illegal stack encrypt from function %s: trying the encrypt caller's stack\n", func_name);
			printk(KERN_ERR "ERROR: user_rbp = %p, prev_user_rsp = %p\n", (char *)user_rbp, (char *)prev_user_rsp);
			//return -1;
		}
	}
	
	/*start encrypt*/
	csit = kmalloc(sizeof(cs_item_t), GFP_KERNEL);
	csit->op = ENCRYPT;
	
	len = strlen(func_name);
	csit->func_name = kmalloc(strlen(func_name) + 1, GFP_KERNEL);
	strncpy(csit->func_name, func_name, len);
	(*(csit->func_name + len)) = '\0';

	csit->user_rbp = user_rbp;
	csit->user_rsp = user_rsp;

	call_stack[call_stack_top] = csit;	
	printk("Insert csit into call stack # %d, uesr_rsp = %p\n", call_stack_top, (char *)csit->user_rsp);
	call_stack_top++;

	//encrypt(user_rsp, user_rbp);
	//for(i = user_rsp; i <= user_rbp; i++){

	prev_rsp = user_rsp;

	stack_buf = sbuf_stack[sbuf_stack_top++];

	for(i = user_rsp; i <= user_rbp;){
		skip = contains(i, para_list);

		if(skip != 0){
			
			if(i > prev_rsp){
				copy_stack_buf(&stack_buf, (char *)prev_rsp, i - prev_rsp);
				memset((char *)prev_rsp, '0', i - prev_rsp);
			}
			i += skip;
			prev_rsp = i;
		}else if(i == user_rbp){
			if(i > prev_rsp){
				copy_stack_buf(&stack_buf, (char *)prev_rsp, i - prev_rsp);
				memset((char *)prev_rsp, '0', i - prev_rsp);
			}
			//(*(char *)i)++;
			i++;
		}else{
			//(*(char *)i)++;
			i++;
		}
	}

	//printk("before restore_pointers\n");	
	restore_pointers(para_list, stack_buf);
	//printk("after restore_pointers\n");	

	//print_stack_buf(stack_buf);
	printk("encrypt from %lx to %lx\n", user_rsp, user_rbp);
	jprobe_return();
	return 0;
}

long jsys_decrypt_stack(void *arg){
	char *func_name;
	register unsigned long kernel_rbp asm("rbp");
	unsigned long user_ip = *((unsigned long *)kernel_rbp + 12);	
	unsigned long user_rsp, user_rbp;
	cs_item_t *csit, *prev_csit;
	sbuf_t *stack_buf;
	
	func_name = get_func_name(mapper, user_ip); 
	/*check*/
	if(call_stack_top == 0){
		printk("Illegal stack decrypt from function %s: no encrypt is found\n", func_name);
		return -1;
	}else{
		prev_csit = call_stack[call_stack_top - 1];
		/*check*/
		if(strcmp(prev_csit->func_name, func_name)){
			printk("Illegal stack decrypt from function %s: functin name does not match\n", func_name);
		}
	}

	
	/*start decrypt*/
	call_stack_top--;
	csit = call_stack[call_stack_top];
	user_rsp = csit->user_rsp;
	user_rbp = csit->user_rbp;

	kfree(csit->func_name);
	kfree(csit);

	
	printk("decrypt from %lx to %lx\n", user_rsp, user_rbp);
	//decrypt(user_rsp, user_rbp);
	//for(i = user_rsp; i <= user_rbp;){
		/*
		skip = contains(i, para_list);

                if(skip != 0){
                        i += skip;
                }else{
                        (*(char *)i)--;
                        i++;
                }
		*/
		stack_buf = sbuf_stack[--sbuf_stack_top];
		restore_stack_buf(stack_buf);
	//	i++;
	
	//}
	jprobe_return();
	return 0;
}
static struct jprobe jvfs_write_probe = {
	.entry			= jvfs_write,
	.kp = {
		.symbol_name	= "vfs_write",
	},
};
static struct jprobe jgeneric_perform_write_probe = {
	.entry			= jgeneric_perform_write,
	.kp = {
		.symbol_name	= "generic_perform_write",
	},
};
static struct jprobe jiov_iter_copy_from_user_atomic_probe = {
	.entry			= jiov_iter_copy_from_user_atomic,
	.kp = {
		.symbol_name	= "iov_iter_copy_from_user_atomic",
	},
};
static struct jprobe jmark_page_accessed_probe = {
	.entry			= jmark_page_accessed,
	.kp = {
		.symbol_name	= "mark_page_accessed",
	},
};
static struct jprobe jsys_encrypt_stack_probe = {
	.entry			= jsys_encrypt_stack,
	.kp = {
		.symbol_name	= "sys_encrypt_stack",
	},
};
static struct jprobe jsys_decrypt_stack_probe = {
	.entry			= jsys_decrypt_stack,
	.kp = {
		.symbol_name	= "sys_decrypt_stack",
	},
};
static int __init jprobe_init(void)
{
	int ret;
	/*
	ret = register_jprobe(&jvfs_write_probe);
	if (ret < 0) {
		printk(KERN_INFO "register_jvfs_write_probe failed, returned %d\n", ret);
		return -1;
	}
	ret = register_jprobe(&jgeneric_perform_write_probe);
	if (ret < 0) {
		printk(KERN_INFO "register_jgeneric_perform_write_probe failed, returned %d\n", ret);
		return -1;
	}
	ret = register_jprobe(&jiov_iter_copy_from_user_atomic_probe);
	if (ret < 0) {
		printk(KERN_INFO "register_jiov_iter_copy_from_user_atomic_probe failed, returned %d\n", ret);
		return -1;
	}
	ret = register_jprobe(&jmark_page_accessed_probe);
	if (ret < 0) {
		printk(KERN_INFO "register_jmark_page_accessed_probe failed, returned %d\n", ret);
		return -1;
	}
	*/
	ret = register_jprobe(&jsys_encrypt_stack_probe);
	if (ret < 0) {
		printk(KERN_INFO "register_jsys_encrypt_stack_probe failed, returned %d\n", ret);
		return -1;
	}

	ret = register_jprobe(&jsys_decrypt_stack_probe);
	if (ret < 0) {
		printk(KERN_INFO "register_jsys_decrypt_stack_probe failed, returned %d\n", ret);
		return -1;
	}
	printk(KERN_INFO "Planted handlers successfully\n");
	/*
	printk(KERN_INFO "Planted jgeneric_perform_write_probe at %p, handler addr %p\n",
	       jgeneric_perform_write_probe.kp.addr, jgeneric_perform_write_probe.entry);
	printk(KERN_INFO "Planted jiov_iter_copy_from_user_atomic_probe at %p, handler addr %p\n",
	       jiov_iter_copy_from_user_atomic_probe.kp.addr, jiov_iter_copy_from_user_atomic_probe.entry);
	*/
	return 0;
}

static void __exit jprobe_exit(void)
{
	/*
	unregister_jprobe(&jgeneric_perform_write_probe);
	unregister_jprobe(&jiov_iter_copy_from_user_atomic_probe);
	unregister_jprobe(&jmark_page_accessed_probe);
	unregister_jprobe(&jvfs_write_probe);
	*/
	unregister_jprobe(&jsys_encrypt_stack_probe);
	unregister_jprobe(&jsys_decrypt_stack_probe);
	printk(KERN_INFO "handlers unregistered\n");
	//printk(KERN_INFO "jprobe at %p unregistered\n", jgeneric_perform_write_probe.kp.addr);
	//printk(KERN_INFO "jprobe at %p unregistered\n", jiov_iter_copy_from_user_atomic_probe.kp.addr);
}

module_init(jprobe_init)
module_exit(jprobe_exit)
MODULE_LICENSE("GPL");
