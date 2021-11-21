#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

//socket libraries:
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <pthread.h>

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

#define BUFSZ 500

int main(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv);
	}

	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1) {
		logexit("socket");
	}
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	printf("connected to %s\n", addrstr);
	
	char buf[BUFSZ];
	memset(buf, 0, BUFSZ);
	
	size_t count;
	unsigned total = 0;

	while(1){
		printf("\n>>");
		fgets(buf, BUFSZ-1, stdin); 

		count = send(s, buf, strlen(buf), 0); //doesn't send '\0' character
		if (count != strlen(buf)) {
			logexit("send");  
		}

		memset(buf, 0, BUFSZ); //clears buffer 
		total = 0;
		count = recv(s, buf + total, BUFSZ - total, 0); //receives data form server
		puts(buf);
		if (count == 0) {
			break; 
		}
		total += count;
		memset(buf,0, BUFSZ);
	}
	close(s); // closes socket
	
	printf("received %u bytes\n", total); 

	exit(EXIT_SUCCESS);
}
