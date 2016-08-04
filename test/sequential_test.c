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


int f1(int x, int y) {
    	register unsigned long long rbp asm("rbp");

	int z ;
	z = x + y ;
	return z ;
}

void f2(){
	int c;
        register unsigned long rbp asm("rbp");

        printf("f2 scan the stack of f1\n");
        for(c = 0; c < 1024; c++){
                printf("%02x ", *((char *)rbp - c)&0XFF);
        }
        printf("\n\n\n");
}

int main () {
	int b ;

	b = f1(35, 64) ;

	f2();

	printf("b = %d\n", b);
	return b;
}
