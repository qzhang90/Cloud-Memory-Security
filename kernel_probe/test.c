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
long ret;
para_list_t *para_list;


unsigned long long getrsp(){
  __asm__ ("movq %rsp, %rax");
}


void f2(int a, int b){
	int c;
	printf("in f2, a = %d, b = %d\n", a, b);
	c = a + b;
}

int f1(void){
	char buff[512];
	int i, j, arg1 = 10, arg2 = 20;
	for(i = 0; i < 512; i++)
	    buff[i] = 'a';
	FILE *f1 = fopen("text.txt", "wb");
	int r1 = fwrite(buff, 1, 512, f1);
	para_t *tmp_para_t;

	fclose(f1);
	int ret = 0, tmp = 123;
	register unsigned long rbp asm("rbp");	
	register unsigned long rsp asm("rsp");	

	printf("fopen, before syscall, rbp = %p, rsp = %p\n", (unsigned long *)rbp, (unsigned long *)getrsp());
	
	/*A malicious user modifies rbp/rsp before the system call*/	
	//__asm__ ("add $0x100, %rbp");
	
	/*Use the assembly code to invoke the system call directly,
	*so that all EIP on the kernel stack will be the EIP of the caller (e.g. f1())
	*/        
	

#ifdef MEMCLEAR
	para_list = malloc(sizeof(para_list_t));
	para_list->size = 2;
	para_list->plist = malloc(para_list->size*sizeof(para_t));
	
	tmp_para_t = para_list->plist + 0;
	tmp_para_t->addr = (unsigned long)&arg1;
	tmp_para_t->size = sizeof(arg1);	
	
	tmp_para_t = para_list->plist + 1;
	tmp_para_t->addr = (unsigned long)&arg2;
	tmp_para_t->size = (unsigned long)sizeof(arg2);	
	
	__asm__ volatile (__syscall
                : "=a" (ret)
                : "0" (316), "D" (para_list)
		: __syscall_clobber );
#endif
	/*A malicious user resumes rbp/rsp after the system call*/	
	//__asm__ ("sub $0x100, %rbp");
	
	register unsigned long rbp1 asm("rbp");	
	register unsigned long rsp1 asm("rsp");	
	
	printf("fopen, after syscall, rbp = %p, rsp = %p\n", (unsigned long *)rbp1, (unsigned long *)getrsp());
	f2(arg1, arg2);
	printf("ret = %d\n", ret);
#ifdef MEMCLEAR
	__asm__ volatile (__syscall
                : "=a" (ret)
                : "0" (317), "D" (para_list) 
		: __syscall_clobber );
	free(para_list);
#endif
	//ret = f2();
	printf("f1 finish\n");
	return ret;
}


void test_call(){
	int (*func)(void) = f1;
	(*func)();
}

int main(void){
	test_call();
	printf("finish\n");
}
