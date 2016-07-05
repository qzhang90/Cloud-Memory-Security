#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "echo_socket"

int request_encrypt_stack(size_t pid, void *top_addr, void *bottom_addr)
{
	int s, t, len;
	struct sockaddr_un remote;
	char str[100];

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	printf("Trying to connect...\n");

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(s, (struct sockaddr *)&remote, len) == -1) {
		perror("connect");
		exit(1);
	}

	printf("Connected.\n");

	//while(printf("> "), fgets(str, 100, stdin), !feof(stdin)) {
	sprintf(str, "e f1 %d %lld %lld\n", pid, (unsigned long)top_addr, (unsigned long)bottom_addr);
	if (send(s, str, strlen(str), 0) == -1) {
		perror("send");
	    	exit(1);
	}

	if ((t=recv(s, str, 100, 0)) > 0) {
	   	 str[t] = '\0';
	   	 printf("echo> %s\n", str);
	} else {
	    	if (t < 0) perror("recv");
	    	else printf("Server closed connection\n");
	    	exit(1);
	}
	//}

	close(s);

	return 0;
}

int main()
{
        int j;
        scanf("%d",&j);
        const int i = j;
        char arr[i];
        unsigned long idx;
	pid_t pid;

        for(j = 0; j < i; j++)
                arr[j] = 'a';

        for(j = 0; j < i; j++)
                printf("%c", arr[j]);
        printf("\n\n\n");
        register unsigned long rsp asm("rsp");
        register unsigned long rbp asm("rbp");

        printf("pid = %d, rsp = %ld, rbp = %ld\n", getpid(), rsp, rbp);
        //printf("rsp = %ld, rbp = %ld\n", rsp, rbp);


	request_encrypt_stack(pid + 2, (char *)rsp, (char *)rbp);
        sleep(20000);


        /*
        for(idx = rsp; idx < rbp;){
                printf("%02x %02x %02x %02x\n", *(char *)idx&0xff, *((char *)idx + 1)&0xff, *((char *)idx + 2)&0xff, *((char *)idx + 3)&0xff);
                idx += 4;
        }
        */

        return 0;
}
