#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

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
	for(i = rbp + 64; i < pre_rbp - 64; i++){
		*((char *)i) = '0';
	}
	
}


int foo(int x, int y) {
    	register unsigned long long rbp asm("rbp");
	int ret;

	/*
	ret = mprotect((char *)rbp - 16, 16, PROT_NONE);
	if(ret)
		printf("in foo, rbp = %x, ret = %s\n", rbp, strerror(errno));
	*/

	int z ;
	z = x + y ;
	return z ;
}

int main () {
	int b ;

	b = foo(35, 64) ;

	printf("b = %d\n", b);
	return b;
}
