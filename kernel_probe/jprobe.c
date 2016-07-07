#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/aio.h>
#include <linux/string.h>

static long jvfs_write(struct file *file, const char __user *buf, size_t count, loff_t *pos){
        char *file_name = file->f_path.dentry->d_iname;

	
        //if((strcmp(file_name, "syslog") != 0) && (strcmp(file_name, "kern.log") != 0))
        if(!strcmp(file_name, "text.txt"))  
	      printk(KERN_INFO "jprobe: jvfs_write, %s, file->f_op->write = %pF\n", file_name, file->f_op->write);

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
        if(!strcmp(file_name, "text.txt"))
		printk(KERN_INFO "jprobe: generic_file_buffered_write,%s, len = %d, pid = %d\n", file_name, iov->iov_len, current->pid);
	jprobe_return();
	return 0;
}


static ssize_t jgeneric_perform_write(struct file *file,
				struct iov_iter *i, loff_t pos){

	char *file_name = file->f_path.dentry->d_iname;
	
	
        if(!strcmp(file_name, "text.txt"))  
		printk(KERN_INFO "jprobe: generic_perform_write, %s, write_begin = %pF, pid = %d\n", file_name, file->f_mapping->a_ops->write_begin, current->pid);
	
	jprobe_return();
	return 0;
}

static struct jprobe my_jprobe = {
	.entry			= jgeneric_perform_write,
	.kp = {
		.symbol_name	= "generic_perform_write",
	},
};

static int __init jprobe_init(void)
{
	int ret;

	ret = register_jprobe(&my_jprobe);
	if (ret < 0) {
		printk(KERN_INFO "register_jprobe failed, returned %d\n", ret);
		return -1;
	}
	printk(KERN_INFO "Planted jprobe at %p, handler addr %p\n",
	       my_jprobe.kp.addr, my_jprobe.entry);
	return 0;
}

static void __exit jprobe_exit(void)
{
	unregister_jprobe(&my_jprobe);
	printk(KERN_INFO "jprobe at %p unregistered\n", my_jprobe.kp.addr);
}

module_init(jprobe_init)
module_exit(jprobe_exit)
MODULE_LICENSE("GPL");
