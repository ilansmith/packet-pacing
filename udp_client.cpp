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
#include <math.h>

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
	int sockfd, n, rc;
	socklen_t salen, salen_local;
	struct sockaddr *sa, *sa_local;
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
        my_getaddrinfo("5.5.0.1", "0", &sockfd, &sa_local, &salen_local);


	if (argc == 4) {
		uint64_t rate = atol(argv[3]);

		if (setsockopt(sockfd, SOL_SOCKET, SO_MAX_PACING_RATE, &rate,
				sizeof(rate))) {
			printf("setsockopt() failed \n");
		}
		printf("settings rate to %ul\n", rate);
	}

	uint64_t total_bytes = 0L;
	float dummy_ratio = (800.006696 - 672) / 800.006696;
	int dummies_sent = 0;
	int next_dummies = 0;
	for (int i = 0; i < NUM_LOOPS; ++i) {
		for (int j = 0; j < 672; ++j) {
			if (sendto(sockfd, buffer, CHUNK_SIZE, 0, sa, salen) < 0) {
				/* buffers aren't available locally at the moment,
				 * try again.
				 */
				if (errno == ENOBUFS) {
					printf("sendto error ENOBYFS");
					fflush(stdout);
					continue;
				}
				perror("error sending datagram");
				exit(1);
			}
			if (!(j % (PACKETS_PER_BURST/10))) {
				next_dummies = floor((i * 800.006696 + j) * dummy_ratio - dummies_sent);
				for (int k = 0; k <= next_dummies; ++k) {
					if (sendto(sockfd, buffer, CHUNK_SIZE, 0, sa_local, salen_local) < 0) {
                                		/* buffers aren't available locally at the moment*/
                                		if (errno == ENOBUFS){
                                       			printf("sendto error ENOBYFS");
							fflush(stdout);
                                        		continue;
						}
                                		perror("error sending dummy datagram");
                                		exit(1);
					}
				}
				dummies_sent += next_dummies;
			}
			total_bytes += CHUNK_SIZE;
		}
		usleep(200000);
	}
	printf("\n %lu bytes sent \n", total_bytes);
	free(sa);
	free(sa_local);
	free(buffer);
	close(sockfd);
	return 0;
}
