memClear-objs = jprobe.o parse_elf.o file_ops.o mapper.o encrypt.o
obj-m += memClear.o

all:
	#make CFLAGS+="-O3" -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	#make CFLAGS+="-O3" -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

