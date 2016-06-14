#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>
#include <linux/time.h>

static struct kprobe kp;
static struct timeval start, end;
static int schedule_counter = 0;

safechar(char c)
{
  if (c >= 'a' && c <= 'z') return(c);
  if (c >= 'A' && c <= 'Z') return(c);
  if (c >= '0' && c <= '9') return(c);
  if (c == ' ') return(c);
  if (c == '.') return(c);
  if (c == '-') return(c);
  if (c == ',') return(c);
  if (c == '(') return(c);
  if (c == ')') return(c);
  if (c == '[') return(c);
  if (c == ']') return(c);
  if (c == '{') return(c);
  if (c == '}') return(c);
  return '@';
}


int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	unsigned long long i;
        char array[512];

	register unsigned long long rsp asm("rsp");
    	register unsigned long long rbp asm("rbp");
    	printk("handler_pre: rsp = %llx, rbp = %llx\n", rsp, rbp);
        for(i = rsp - 4096; i < rbp; i++){
                printk("%x ", *(char *)i);
        }

	printk("\nhandler_pre\n");    
	return 0;
}

void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{	
	unsigned long long i;
        char array[512];


	register unsigned long long rsp asm("rsp");
    	register unsigned long long rbp asm("rbp");
    	printk("handler_post: rsp = %llx, rbp = %llx\n", rsp, rbp);
        for(i = rsp - 4096; i < rbp; i++){
                printk("%x ", *(char *)i);
        }

	printk("\nhandler_post\n");    
}

int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
	int i;
        char array[512];

        for(i = 0; i < 512; i++)
             printk("%c ", safechar(array[i]));
    printk("\nA fault happened during probing.\n");
    return 0;
}

int init_module(void)
{
    int ret;

    kp.pre_handler = handler_pre;
    kp.post_handler = handler_post;
    kp.fault_handler = handler_fault;
    kp.addr = (kprobe_opcode_t*) kallsyms_lookup_name("ext4_mkdir");
    
    if (!kp.addr) {
        printk("Couldn't get the address of schedule.\n");
        return -1;
    }

    if ((ret = register_kprobe(&kp) < 0)) {
        printk("register_kprobe failed, returned %d\n", ret);
        return -1;
    }


    printk("kprobe registered\n");
    return 0;
}

void cleanup_module(void)
{
    unregister_kprobe(&kp);
    printk("kprobe unregistered\n");
}

MODULE_LICENSE("GPL");