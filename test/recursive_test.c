#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/ptrace.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <unistd.h>
#include </usr/src/linux-3.14.4-memClear/include/linux/memclear_para.h>

//#define _syscall1(NR) stub_syscall1(NR)
#define __syscall_clobber "r11","rcx","memory"
#define __syscall "syscall"

#define MEMCLEAR	1
static int n = 0;

int ret;
para_list_t *para_list;


void __attribute__((__no_instrument_function__))
__cyg_profile_func_enter(void *this_func, void *call_site)
{
}

void __attribute__((__no_instrument_function__))
__cyg_profile_func_exit(void *this_func, void *call_site)
{
        register unsigned long long rsp asm("rsp");
        register unsigned long long rbp asm("rbp");

        unsigned long long pre_rbp, i;

        pre_rbp = *(unsigned long long *)rbp;
        for(i = rbp + 16; i < pre_rbp - 16; i++){
                *((char *)i) = '0';
        }

}



int f2(int a, int b, int *p){
	int c, x;
	register unsigned long rbp asm("rbp");
	
	printf("f2 scan the stack of f1\n");
	for(c = 0; c < 1024; c++){
                printf("%02x ", *((char *)rbp + c)&0XFF);
        }
	printf("\n\n\n\n");	

	*p += 1;
	c = a + b;
	f1();
	return c;
}

int f1(void){
	char buff[1024];
	int i, arg1 = 10, arg2 = 20;
	para_t *tmp_para_t;
	int *u;

	int ret = 0, tmp = 123;

	printf("f1 has been called\n");
	n++;
	if(n == 2){
		return 0;
	}

	
	/*A malicious user modifies rbp/rsp before the system call*/	
	//__asm__ ("add $0x100, %rbp");
	
	/*Use the assembly code to invoke the system call directly,
	*so that all EIP on the kernel stack will be the EIP of the caller (e.g. f1())
	*/        

	int k[5] = {1,2,3,4,5};	
	
	for(i = 0; i < 1024; i++)
	    buff[i] = 'a';

	u = k;
#ifdef MEMCLEAR
	para_list = malloc(sizeof(para_list_t));
	para_list->size = 3;
	para_list->plist = malloc(para_list->size*sizeof(para_t));
	
	tmp_para_t = para_list->plist + 0;
	tmp_para_t->addr = (unsigned long)&arg1;
	tmp_para_t->size = sizeof(arg1);	
	tmp_para_t->pointer = 0;	
	
	tmp_para_t = para_list->plist + 1;
	tmp_para_t->addr = (unsigned long)&arg2;
	tmp_para_t->size = (unsigned long)sizeof(arg2);	
	tmp_para_t->pointer = 0;	

	tmp_para_t = para_list->plist + 2;
	tmp_para_t->addr = (unsigned long)&u;
	tmp_para_t->size = (unsigned long)(5*sizeof(int));
	tmp_para_t->pointer = 1;

	register unsigned long rbp asm("rbp");
	register unsigned long rsp asm("rsp");

	/*BUG: without this printf, rsp will be larger than rbp in encrypt_stack.*/
	printf("usre_rbp = %p, user_rsp = %p\n", (char *)rbp, (char *)rsp);

	__asm__ volatile (__syscall
                : "=a" (ret)
                : "0" (316), "D" (para_list)
		: __syscall_clobber );
#endif
	/*A malicious user resumes rbp/rsp after the system call*/	
	//__asm__ ("sub $0x100, %rbp");
	
	ret = f2(arg1, arg2, u);
	printf("ret = %d, k[0] = %d\n", ret, k[0]);
#ifdef MEMCLEAR
	__asm__ volatile (__syscall
                : "=a" (ret)
                : "0" (317), "D" (para_list) 
		: __syscall_clobber );
	free(para_list);
#endif
	return ret;
}



int main(void){
	f1();
	printf("finish\n");
}
