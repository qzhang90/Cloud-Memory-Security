memClear-objs = jprobe.o parse_elf.o file_ops.o mapper.o stack_buf.o stack_encrypt.o
obj-m += memClear.o

all:
	#make CFLAGS+="-O3" -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	#make CFLAGS+="-O3" -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f *.mod.c modules.order Module.symvers *.ko

