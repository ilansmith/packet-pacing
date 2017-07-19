#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <errno.h>

#include "buff_size.h"



void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
 int sockfd, n, rc;
  socklen_t salen;
  struct sockaddr *sa;
  struct addrinfo hints, *res, *ressave;
  char *host, *port ;
  int i;
  int fromlen;
  int ackvar;
  int nonacks,acks;
  char* buffer = new char[CHUNK_SIZE];

  nonacks=0;
  acks=0;
 
  if(argc < 3)
   { printf("usage: %s <hostname/IPaddress> <portnumber> [rate] \n",argv[0]);
     exit(0);
   }

  host=argv[1];
  port=argv[2];
 
  /*initilize addrinfo structure*/
  bzero(&hints, sizeof(struct addrinfo));  
  hints.ai_family=AF_UNSPEC;
  hints.ai_socktype=SOCK_DGRAM;
  hints.ai_protocol=IPPROTO_UDP;

  if((n=getaddrinfo(host, port, &hints, &res)) != 0)
    printf("udpclient error for %s, %s: %s", host, port, gai_strerror(n));
  ressave=res;

  do{/* each of the returned IP address is tried*/
    sockfd=socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(sockfd>= 0)
      break; /*success*/
  }while ((res=res->ai_next) != NULL);

  sa=(struct sockaddr *)  malloc(res->ai_addrlen);
  memcpy(sa, res->ai_addr, res->ai_addrlen);
  salen=res->ai_addrlen;

  freeaddrinfo(ressave);
 

    uint64_t rate;

    if(argc > 3) {
         rate = atol(argv[3]);
         int val1 = setsockopt(sockfd, SOL_SOCKET, SO_MAX_PACING_RATE, &rate, sizeof(rate));
         printf("settings rate to %ul\n", rate);
    }

    uint64_t total_bytes = 0L;
    int bytes; 
    for(int ii = 0 ; ii < NUM_LOOPS ; ++ii)  {
       	if( bytes = sendto(sockfd, buffer, CHUNK_SIZE, 0,sa, salen) <0 ) {
	/* buffers aren't available locally at the moment,
	 * try again.
	 */
	if (errno == ENOBUFS)
		continue;
	perror("error sending datagram");
	exit(1);
        }
       total_bytes+=CHUNK_SIZE;
    }
    printf("\n %lu bytes sent \n", total_bytes);
    close(sockfd);
    return 0;
}
