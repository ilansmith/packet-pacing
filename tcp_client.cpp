#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include "buff_size.h"



#define DATA_SIZE (1024*10)

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char *buffer = new char[CHUNK_SIZE];
    if (argc < 3) {
       printf("usage: %s <hostname/IPaddress> <portnumber> [rate] \n",argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    uint64_t rate;



    if(argc > 3) {
         rate = atol(argv[3]);
         int val1 = setsockopt(sockfd, SOL_SOCKET, SO_MAX_PACING_RATE, &rate, sizeof(rate));
         printf("settings rate to %ul\n", rate);
    }


    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    uint64_t total_bytes = 0L;
    for(int ii = 0 ; ii < NUM_LOOPS ; ++ii)  {    
        n = write(sockfd, buffer, CHUNK_SIZE);
        if (n < 0) 
            error("ERROR writing to socket"); 
        total_bytes+=n; 
    }
    printf("\n%lu bytes sent\n", total_bytes);
    close(sockfd);
    return 0;
}
