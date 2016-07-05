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

#define SOCK_PATH "echo_socket"

#define PAGEMAP_LENGTH 8

typedef struct stack_area{
	unsigned long page_frame;
	unsigned long offset;
	unsigned long len;
}sarea;

safechar(char c)
{
  if (c >= 'a' && c <= 'z') return(c);
  if (c >= 'A' && c <= 'Z') return(c);
  if (c >= '0' && c <= '9') return(c);
  if (c == ' ') return(c);
  if (c == '.') return(c);
  if (c == '-') return(c);
  if (c == ',') return(c);
  if (c == '(') return(c);
  if (c == ')') return(c);
  if (c == '[') return(c);
  if (c == ']') return(c);
  if (c == '{') return(c);
  if (c == '}') return(c);
  return '@';
}

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

	sprintf(path_to_pagemap, "/proc/%d/pagemap", pid);
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

/*
*Get the physical page areas allocated to the stack
*pid: pid of the process
*top_addr: virtual address of the stack top
*bottom_addr: virtual address of the stack bottom
*return: a list of physical page areas
*/
sarea *get_stack_areas(size_t pid, void *top_addr, void *bottom_addr){
	sarea *tmp_sarea, *ret;
	int i, j;
	char *tmp_addr;
	unsigned long page_num, pre_page_num, offset;

	ret = (sarea *)malloc(sizeof(sarea)*10);
	
	for(i = 0; i < 10; i++){
		(ret + i)->len = -1;
	}

	pre_page_num = page_num = get_page_frame_number_of_address(pid, top_addr);
	offset = ((unsigned long)top_addr)%4096;

	i = 0;	
	for(tmp_addr = (char *)top_addr + 1; tmp_addr < (char *)bottom_addr; tmp_addr++){
		page_num = get_page_frame_number_of_address(pid, tmp_addr);
                if(page_num != pre_page_num){
              		tmp_sarea = ret + i;
			tmp_sarea->page_frame = pre_page_num;
			tmp_sarea->offset = offset; 
			tmp_sarea->len = (char *)tmp_addr - (char *)top_addr;
			top_addr = tmp_addr;
			
			i++;
			pre_page_num = page_num;
			offset = ((unsigned long)tmp_addr)%4096;	
		}
        }

	//Append the last stack area
	tmp_sarea = ret + i;
	tmp_sarea->page_frame = pre_page_num;
	tmp_sarea->offset = offset; 
	tmp_sarea->len = (char *)tmp_addr - (char *)top_addr;
	
	return ret;
}


/*
Print the content of a physical page
frame_num: frame number of the physical page
return: error if -1; successful if > 0
*/
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
                printf("%2c", safechar(buffer[i]));
        }
        printf("\n\n\n");

        return fd;
}

int encrypt_stack(size_t pid, void *top_addr, void *bottom_addr){
	sarea *s, *tmp;
	int i, j;
	char buffer[4096];
	int fd, pos;

	s = get_stack_areas(pid, top_addr, bottom_addr);
	for(i = 0; i < 10; i++){
		tmp = s + i;
		if(tmp->len != -1){
			fd = open("/dev/mem", O_RDWR);

			if(fd == -1) {
				fprintf(stderr, "Error opening /dev/mem: %s\n", strerror(errno));
				return fd;
			}

			pos = lseek(fd, tmp->page_frame*4096, SEEK_SET);

			if(pos == -1) {
				fprintf(stderr, "Failed to seek /dev/mem: %s\n", strerror(errno));
				return pos;
			}
	
			read(fd, buffer, 4096);

			for(j = tmp->offset; j < tmp->offset + tmp->len; j++){
				//Encryption algorithm
			}

			write(fd, buffer, 4096);
						
        		close(fd);
		}
	}
	
	return 0;
}

int main(void)
{
	int s, s2, t, len;
	struct sockaddr_un local, remote;
	char str[100];
	char *p, *func_name;
	char op; //e - encrypt, d - decrypt
	size_t pid;
	unsigned long rsp, rbp;

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCK_PATH);
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(s, (struct sockaddr *)&local, len) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(s, 5) == -1) {
		perror("listen");
		exit(1);
	}

	for(;;) {
		int n;
		printf("Waiting for a connection...\n");
		t = sizeof(remote);
		if ((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1) {
			perror("accept");
			exit(1);
		}

		printf("Connected.\n");

		n = recv(s2, str, 100, 0);
		printf("n = %d\n", n);
		if (n <= 0) {
			if (n < 0) perror("recv");
		}

		/*e.g. "e/d f1 1234(pid) 123456(rsp) 125839(rbp)"*/

		p = strtok(str, " ");
		op = *p;

		p = strtok(NULL, " "); //get the name of the function, who wants to encrypt its stack
		len = strlen(p) + 1;
		func_name = malloc(len);
		strcpy(func_name, p);
		*(func_name + len - 1) = '\0';
		
		p = strtok(NULL, " "); //get the pid of the process
		pid = atol(p);
		

		p = strtok(NULL, " "); //get rsp of the function, who wants to encrypt its stack
		rsp = atol(p);
	
		p = strtok(NULL, " "); //get rbp of the function, who wants to encrypt its stack
		rbp = atol(p);
		
		if('e' == *p){
			//encrypt the stack
			encrypt_stack(pid, (char *)rsp, (char *)rbp);
		}else if('d' == *p){
			//decrypt the stack
		}


		if (send(s2, str, n, 0) < 0) {
			perror("send");
		}

		close(s2);
	}

	return 0;
}
