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
#include <chrono>
#include <time.h>


#define LOCAL_IP ("4.4.0.1")
#define OUT_PORT_LOCAL ("20654")

#define USEC_RESOLUTION (200000) /* in usec */

#define USE_MTU_PACKETS (0)
#define USE_DUMMY (0)

/* Headers: 42 bytes */
#if USE_MTU_PACKETS
#define MAX_PACING_RATE (5775500)
#define EXACT_USER_PACKET_PER_BURST (640.863612781)
#define TOTAL_PACKET_PER_BURST (800.006695905)
#define CHUNK_SIZE (1472) /*1514-42*/
#else
#define MAX_PACING_RATE (5172750)
#define EXACT_USER_PACKET_PER_BURST (715.536511615)
#define TOTAL_PACKET_PER_BURST (800.003171681)
#define CHUNK_SIZE (1314) /*1356-42*/
#endif

#define NUM_LOOPS (100000000L)

#define DUMMY_SEND_FLAG (0x400) /* equals to MSG_SYN */
#define DUMMY_SPOOF_MAC (0x44000000)

#define NOW() std::chrono::high_resolution_clock::now()
#define SUB_MICRO(X, Y) std::chrono::duration_cast<std::chrono::microseconds>(X - Y).count()

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

static int run(int sockfd, struct sockaddr *sa, struct sockaddr *sa_local, socklen_t salen, socklen_t salen_local)
{
	char *buffer;
	uint64_t total_bytes = 0L;
	int ret = -1;
	float dummy_ratio = (TOTAL_PACKET_PER_BURST - EXACT_USER_PACKET_PER_BURST) / TOTAL_PACKET_PER_BURST;
	float counter_next_dummy = 0;
	int dummies_sent = 0;
	int next_dummies = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> start = NOW();
	std::chrono::time_point<std::chrono::high_resolution_clock> end;
	std::chrono::microseconds resolution(USEC_RESOLUTION);

	if (!(buffer = (char*)malloc(CHUNK_SIZE))) {
		printf("Could not allocate data buffer\n");
		return -1;
	}

	for (int i = 0; i < NUM_LOOPS; ++i) {
		float next_stop_send = (i+1) * EXACT_USER_PACKET_PER_BURST;
		for (uint32_t j = floor(i * EXACT_USER_PACKET_PER_BURST);
				j < next_stop_send;
				++j) {
			if (sendto(sockfd, buffer, CHUNK_SIZE, 0, sa, salen) < 0) {
				/* buffers aren't available locally at the moment */
				if (errno == ENOBUFS) {
					printf("sendto error ENOBYFS");
					fflush(stdout);
					continue;
				}
				perror("error sending datagram");
				ret = -1;
				goto Exit;
			}

			counter_next_dummy += dummy_ratio;
			if (USE_DUMMY && dummy_ratio > 1.0) {
				counter_next_dummy -= 1.0;
				if (sendto(sockfd, buffer, CHUNK_SIZE, 0, sa, salen/*sa_local, salen_local*/) < 0) {
					// buffers aren't available locally at the moment
					if (errno == ENOBUFS){
						printf("sendto error ENOBYFS");
						fflush(stdout);
						continue;
					}
					perror("error sending dummy datagram");
					ret = -1;
					goto Exit;
				}
			}
			total_bytes += CHUNK_SIZE;
		}
		end = NOW();
		usleep(USEC_RESOLUTION - SUB_MICRO(end, start) - 10/* slightly reduce sleep time */);

		while (start + resolution > NOW()); /* busy wait for the correct time */
		start = start + resolution;
	}
	printf("\n %lu bytes sent \n", total_bytes);
	ret = 0;

Exit:
	free(buffer);
	return ret;
}

int main(int argc, char *argv[]) {
	int sockfd = -1, rc;
	socklen_t salen, salen_local;
	struct sockaddr *sa = NULL, *sa_local = NULL;
	char *host, *port;
	uint64_t rate = MAX_PACING_RATE;

	int ret = -1;

	if (argc < 3 || argc > 4) {
		printf("usage: %s <hostname/IPaddress> <portnumber> [rate] \n", argv[0]);
		exit(-1);
	}

	host = argv[1];
	port = argv[2];

	if (my_getaddrinfo(host, port, &sockfd, &sa, &salen)){
		goto Exit;
	}
	if (my_getaddrinfo(LOCAL_IP, OUT_PORT_LOCAL, NULL, &sa_local, &salen_local)) {
		goto Exit;
	}


	printf("settings rate to: %lu\n", rate);
	if (setsockopt(sockfd, SOL_SOCKET, SO_MAX_PACING_RATE, &rate, sizeof(rate))) {
		printf("setsockopt() failed \n");
		goto Exit;
	}

	if (run(sockfd, sa, sa_local, salen, salen_local) < 0) {
		printf("program exit on error.\n");
		goto Exit;
	}

	ret = 0;

Exit:
	free(sa);
	free(sa_local);
	if (-1 < sockfd)
		close(sockfd);
	return ret;
}
