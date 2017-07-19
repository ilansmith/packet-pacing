#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <iostream>
#include <errno.h>

#include "buff_size.h"

#define DUMMY_SEND_FLAG 0x400 // equals to MSG_SYN
#ifndef SO_MAX_PACING_RATE
#define SO_MAX_PACING_RATE 47
#endif

void error(const char *msg) {
	perror(msg);
	exit(0);
}

static int my_getaddrinfo(const char *host, const char *port, int *sockfd,
	struct sockaddr **sa, socklen_t *salen)
{
	struct addrinfo hints, *res, *ressave;
	int n, ret = -1;

	/*initilize addrinfo structure*/
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	if ((n = getaddrinfo(host, port, &hints, &res)) != 0) {
		printf("udpclient error for %s, %s: %s", host, port,
			gai_strerror(n));
		return -1;
	}

	ressave = res;
	if (sockfd) {
		do {/* each of the returned IP address is tried*/
			int tmp;

			tmp = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
			if (tmp >= 0) {
				printf("inet_ntoa(((struct sockaddr_in*)res->ai_addr)->sin_addr): %s\n",
						inet_ntoa(((struct sockaddr_in*)res->ai_addr)->sin_addr));
				*sockfd = tmp;
				break; /*success*/
			}
		} while ((res = res->ai_next) != NULL);

		if (!res) {
			printf("Could not open a socket for %s:%s\n", host, port);
			goto Exit;
		}
	}

	*sa = (struct sockaddr*)malloc(res->ai_addrlen);
	memcpy(*sa, res->ai_addr, res->ai_addrlen);
	*salen = res->ai_addrlen;

	ret = 0;
Exit:
	freeaddrinfo(ressave);
	return ret;
}

int main(int argc, char *argv[])
{
	int sockfd, n, rc, i;
	socklen_t salen;
	struct sockaddr *sa;
	char *host, *port;
	char *buffer;

	if (!(buffer = (char*)malloc(CHUNK_SIZE))) {
		printf("Could not allocate data buffer\n");
		return -1;
	}

	if (argc < 3 || argc > 4) {
		printf("usage: %s <hostname/IPaddress> <portnumber> [rate] \n",
				argv[0]);
		exit(-1);
	}

	host = argv[1];
	port = argv[2];

	my_getaddrinfo(host, port, &sockfd, &sa, &salen);

	if (argc == 4) {
		uint64_t rate = atol(argv[3]);

		if (setsockopt(sockfd, SOL_SOCKET, SO_MAX_PACING_RATE, &rate,
				sizeof(rate))) {
			printf("setsockopt() failed \n");
		}
		printf("settings rate to %ul\n", rate);
	}

	uint64_t total_bytes = 0L;
	int bytes;
	for (int i = 0; i < NUM_LOOPS; ++i) {
		int j;

		for (j = 0; j < 763; j++) {
			if (bytes = sendto(sockfd, buffer, CHUNK_SIZE, 0, sa, salen) < 0) {
				/* buffers aren't available locally at the moment,
				 * try again.
				 */
				if (errno == ENOBUFS)
					continue;
				perror("error sending datagram");
				exit(1);
			}
			total_bytes += CHUNK_SIZE;
		}
		usleep(200000);
	}
	printf("\n %lu bytes sent \n", total_bytes);
	free(sa);
	free(buffer);
	close(sockfd);
	return 0;
}
