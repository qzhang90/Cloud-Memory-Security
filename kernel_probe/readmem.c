#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PAGEMAP_LENGTH 8

/*
*Compute the physical page frame number for a given virtual address in a given process
*pid: pid of the process
*addr: virtual address of the process
*return: the physical page mapped to addr
Supposing the page size is 4KB
*/
unsigned long get_page_frame_number_of_address(size_t pid, void *addr) {
	char path_to_pagemap[50];
	FILE *pagemap;
	unsigned long offset = 0, page_frame_number = 0;

	sprintf(path_to_pagemap, "/proc/%d/pagemap", (int)pid);


	// Open the pagemap file for the current process
	pagemap = fopen(path_to_pagemap, "rb");

	// Seek to the page that the buffer is on
	offset = (unsigned long)addr / getpagesize() * PAGEMAP_LENGTH;
	if(fseek(pagemap, (unsigned long)offset, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to seek pagemap to proper location\n");
		exit(1);
	}

	// The page frame number is in bits 0-54 so read the first 7 bytes and clear the 55th bit
	fread(&page_frame_number, 1, PAGEMAP_LENGTH-1, pagemap);

	page_frame_number &= 0x7FFFFFFFFFFFFF;

	fclose(pagemap);

	return page_frame_number;
}

/*
*Compute the physical page frame numbers for a range of virtual address in a given process
*pid: pid of the process
*addr: virtual address of the process
*return: the physical pages mapped to [addr. addr + 4KB)
*At most 2 page frame numbers will be returned
*/
unsigned long *get_page_frames(size_t pid, void *addr){
       	char *tmp_addr;
	unsigned long offset, pre_offset;
	unsigned long *ret = malloc(2*sizeof(unsigned long));

	pre_offset = offset = get_page_frame_number_of_address(pid, addr);
	*ret = pre_offset;
	*(ret + 1) = 0;	

	for(tmp_addr = (char *)addr + 1; tmp_addr < (char *)addr + 4096; tmp_addr++){
                offset = get_page_frame_number_of_address(pid, tmp_addr);
                if(offset != pre_offset){
			*(ret + 1) = offset;
			return ret;
                }
        }	
	
	return ret;
}

int read_memory_page(unsigned long frame_num) {
        char buffer[4096];
        int i;
        unsigned long pos;

        // Open the memory (must be root for this)
        int fd = open("/dev/mem", O_RDWR);

        if(fd == -1) {
        	fprintf(stderr, "Error opening /dev/mem: %s\n", strerror(errno));
        	return fd;
        }

        pos = lseek(fd, frame_num*4096, SEEK_SET);

        if(pos == -1) {
                fprintf(stderr, "Failed to seek /dev/mem: %s\n", strerror(errno));
                return pos;
        }

        read(fd, buffer, 4096);

        close(fd);

        for(i = 0; i < 4096; i++){
                printf("%02x ", buffer[i]&0xFF);
        }
        printf("\n\n\n");

        return fd;
}

void main(){
	unsigned long page_frames;

	//page_frames = get_page_frames(14380, 0xffff880043f62000);
	//page_frames = get_page_frame_number_of_address(15853, 0xffff8800437c7000);
	//printf("page_frame = %ld\n", page_frames);
	read_memory_page(340417);
}
