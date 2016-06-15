#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/pagemap.h>
#include <linux/rmap.h>
#include <linux/memory.h>
#include <linux/time.h>
#include <linux/preempt.h>
#include <linux/hardirq.h>


# define op_t	unsigned long int
# define OPSIZ	(sizeof(op_t))

typedef unsigned char byte;

unsigned long long  start, end;

void * glibc_memset (void *dstpp, int c, size_t len)
{
  long int dstp = (long int) dstpp;

  if (len >= 8)
    {
      size_t xlen;
      op_t cccc;

      cccc = (unsigned char) c;
      cccc |= cccc << 8;
      cccc |= cccc << 16;
      if (OPSIZ > 4)
	/* Do the shift in two steps to avoid warning if long has 32 bits.  */
	cccc |= (cccc << 16) << 16;

      /* There are at least some bytes to set.
	 No need to test for LEN == 0 in this alignment loop.  */
      while (dstp % OPSIZ != 0)
	{
	  ((byte *) dstp)[0] = c;
	  dstp += 1;
	  len -= 1;
	}

      /* Write 8 `op_t' per iteration until less than 8 `op_t' remain.  */
      xlen = len / (OPSIZ * 8);
      while (xlen > 0)
	{
	  ((op_t *) dstp)[0] = cccc;
	  ((op_t *) dstp)[1] = cccc;
	  ((op_t *) dstp)[2] = cccc;
	  ((op_t *) dstp)[3] = cccc;
	  ((op_t *) dstp)[4] = cccc;
	  ((op_t *) dstp)[5] = cccc;
	  ((op_t *) dstp)[6] = cccc;
	  ((op_t *) dstp)[7] = cccc;
	  dstp += 8 * OPSIZ;
	  xlen -= 1;
	}
      len %= OPSIZ * 8;

      /* Write 1 `op_t' per iteration until less than OPSIZ bytes remain.  */
      xlen = len / OPSIZ;
      while (xlen > 0)
	{
	  ((op_t *) dstp)[0] = cccc;
	  dstp += OPSIZ;
	  xlen -= 1;
	}
      len %= OPSIZ;
    }

  /* Write the last few bytes.  */
  while (len > 0)
    {
      ((byte *) dstp)[0] = c;
      dstp += 1;
      len -= 1;
    }

  return dstpp;
}



int init_module(void)
{
	int i;	
	unsigned long flags;
	char *p = vmalloc(4096);
	
	memset(p, '0', 4096);
	
        preempt_disable();
        raw_local_irq_save(flags);

	__asm__ volatile ("CPUID\n\t"::: "%rax", "%rbx", "%rcx", "%rdx");
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (start));	
	
	for(i = 0; i < 8192; i++)
                glibc_memset(p, '0', 4096);
	
	__asm__ volatile ("CPUID\n\t"::: "%rax", "%rbx", "%rcx", "%rdx");
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (end));	

        preempt_enable();
        raw_local_irq_restore(flags);

	vfree(p);
	printk("%c\n", *(p + 1024));
	printk("%lld\n", end - start);

	return 0;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Goodbye world 1.\n");
}
