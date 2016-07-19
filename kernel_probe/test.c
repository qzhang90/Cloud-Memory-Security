#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/ptrace.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <unistd.h>

#define _syscall0(NR) stub_syscall0(NR)
#define __syscall_clobber "r11","rcx","memory"
#define __syscall "syscall"
long ret;

unsigned long long getrsp()
{
  __asm__ ("movq %rsp, %rax");
}


int f2(void){
	int a, b, c;
	
	a = 1;
	b = 2;
	c = a + b;
	return c;
}

int f1(void){
	char buff[512];
	int i, j;
	for(i = 0; i < 512; i++)
	    buff[i] = 'a';
	FILE *f1 = fopen("text.txt", "wb");
	int r1 = fwrite(buff, 1, 512, f1);
	
	fclose(f1);
	int ret;
	register unsigned long rbp asm("rbp");	
	register unsigned long rsp asm("rsp");	

	printf("fopen, before syscall, rbp = %p, rsp = %p\n", (unsigned long *)rbp, (unsigned long *)getrsp());
	
	/*A malicious user modifies rbp/rsp before the system call*/	
	//__asm__ ("add $0x100, %rbp");
	
	/*Use the assembly code to invoke the system call directly,
	*so that all EIP on the kernel stack will be the EIP of the caller (e.g. f1())
	*/        
	
	__asm__ volatile (__syscall
                : "=a" (ret)
                : "0" (316) : __syscall_clobber );

	/*A malicious user resumes rbp/rsp after the system call*/	
	//__asm__ ("sub $0x100, %rbp");
	
	register unsigned long rbp1 asm("rbp");	
	register unsigned long rsp1 asm("rsp");	
	
	printf("fopen, after syscall, rbp = %p, rsp = %p\n", (unsigned long *)rbp1, (unsigned long *)getrsp());
	ret = f2();
	//printf("ret = %d\n", ret);
	__asm__ volatile (__syscall
                : "=a" (ret)
                : "0" (317) : __syscall_clobber );
	//ret = f2();
	//printf("ret = %d\n", ret);
	return ret;
}

int main(void){
	f1();
	printf("finish\n");
}
