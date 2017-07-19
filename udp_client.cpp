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
	int sockfd, n, rc;
	socklen_t salen, salen_local;
	struct sockaddr *sa, *sa_local;
	struct sockaddr_in sa_in;
	socklen_t sin_len;
	char *host, *port;
	int i;
	int fromlen;
	int ackvar;
	int nonacks, acks;
	char* buffer = new char[CHUNK_SIZE];

	nonacks = 0;
	acks = 0;

	if (argc < 3 || argc > 4) {
		printf("usage: %s <hostname/IPaddress> <portnumber> [rate] \n",
				argv[0]);
		exit(-1);
	}

	host = argv[1];
	port = argv[2];

	my_getaddrinfo(host, port, &sockfd, &sa, &salen);

	if (argc > 3) {
		uint64_t rate = atol(argv[3]);

		int val1 = setsockopt(sockfd, SOL_SOCKET, SO_MAX_PACING_RATE, &rate,
				sizeof(rate));
		printf("settings rate to %ul\n", rate);
	}

	int rate_packets = 0;
	int required_rate = 0;
	if (argc > 5) {
		rate_packets = atol(argv[4]);
		required_rate = atol(argv[5]);
	}

	uint64_t total_bytes = 0L;
	int bytes;
	int count_packets_rate = 0;
	for (int ii = 0; ii < NUM_LOOPS; ++ii) {
		if (bytes = sendto(sockfd, buffer, CHUNK_SIZE, 0, sa, salen) < 0) {
			/* buffers aren't available locally at the moment,
			 * try again.
			 */
			if (errno != ENOBUFS) {
				perror("error sending datagram");
				exit(1);
			}
			continue;
		}
		count_packets_rate += 1;
		if (count_packets_rate % rate_packets == 0) {
			for (int j = 0; j < required_rate - rate_packets; ++j) {
				bytes = sendto(sockfd, buffer, CHUNK_SIZE, DUMMY_SEND_FLAG,
						sa, salen);
			}
		}
		total_bytes += CHUNK_SIZE;
	}
	printf("\n %lu bytes sent \n", total_bytes);
	free(sa);
	close(sockfd);
	return 0;

}
