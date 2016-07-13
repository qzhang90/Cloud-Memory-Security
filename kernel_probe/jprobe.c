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



pid_t pid = -1;
struct page *p = NULL;

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

long inline jsys_encrypt_stack(void){
	
	struct user_regset_view *view = task_user_regset_view(current);
	const struct user_regset *regset = &view->regsets[REGSET_GENERAL];
	unsigned long regs[27];
	int ret, i;
	unsigned long bp, sp;
	register unsigned long bp1 asm("rbp");
	
	regset->get(current, regset, 0, 27*8, regs, NULL);
	bp = regs[4];
	sp = regs[19];

	printk("bp = %p, sp = %p, bp1 = %p, ret addr = %p\n",(unsigned long *)bp, (unsigned long *)sp, (unsigned long *)bp1, *((unsigned long *)sp));
	
	/*The addresses kernel stack frames we are interested are higher than bp1 */
	for(i = 0; i < 20; i++){
		printk("%p\n", *((unsigned long *)bp1 + i));
	}
	printk("\n\n\n\n\n");
	printk("regs = %p, ret = %d, ip = %x\n", regs, ret, regs[16]);
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
	printk(KERN_INFO "handlers unregistered\n");
	//printk(KERN_INFO "jprobe at %p unregistered\n", jgeneric_perform_write_probe.kp.addr);
	//printk(KERN_INFO "jprobe at %p unregistered\n", jiov_iter_copy_from_user_atomic_probe.kp.addr);
}

module_init(jprobe_init)
module_exit(jprobe_exit)
MODULE_LICENSE("GPL");
