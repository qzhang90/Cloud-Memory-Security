#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/ptrace.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <unistd.h>
int xyz = 0;

#define _syscall0(NR) stub_syscall0(NR)
#define __syscall_clobber "r11","rcx","memory"
#define __syscall "syscall"
long ret;

int foo(){
	return 5;
}

int foo1(){
	return 5;
}

char bar(){
	char tmp;

	tmp = 'a';
	return tmp + 1;
}

int main(void)
{
	
	char buff[512], c, c1;
	int i, j, i1, j1;

	register unsigned long rbp asm("rbp");	
	register unsigned long rsp asm("rsp");	
	printf("rbp = %p, rsp = %p\n", (unsigned long *)rbp, (unsigned long *)rsp);


	/*user the assembly code to invoke the system call directly,
	*so that all EIP on the kernel stack will be the EIP of the main()*/        
	__asm__ volatile (__syscall
                : "=a" (ret)
                : "0" (316) : __syscall_clobber );
	
	for(i = 0; i < 512; i++)
	    buff[i] = 'a';
	FILE *f1 = fopen("text.txt", "wb");
	int r1 = fwrite(buff, 1, 512, f1);
	
	fclose(f1);
	i = foo();
	j = foo1();
	c = bar();
	i1 = foo();
	j1 = foo1();
	c1 = bar();
	xyz++;
	printf("%c, %d, %d\n", c, j, xyz);
	printf("%c, %d, %d\n", c1, j1, xyz);
        
	__asm__ volatile (__syscall
                : "=a" (ret)
                : "0" (316) : __syscall_clobber );
	
	return i;
}
