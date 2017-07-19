#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <chrono>
#include <ctime>
#include <iostream>
#include <errno.h>

#include "buff_size.h"



void printer(uint64_t total_bytes, double total_time)
{
     printf("\n * Total bytes read     = %lu ", total_bytes);
     printf("\n * Total seconds passed = %f ", total_time);
     printf("\n * Total speed          = %f bytes/second", total_bytes/total_time );
     printf("\n * Total speed          = %f KiloBytes/second", total_bytes/total_time/1024 );
     printf("\n * Total speed          = %f MegaBytes/second", total_bytes/total_time/1024/1024 );
     printf("\n * Total speed          = %f GigaBytes/second", total_bytes/total_time/1024/1024/1024 );
     printf("\n -------------------------------------------");
     printf("\n * Total speed          = %f Bits/second", (8*total_bytes)/total_time );
     printf("\n * Total speed          = %f KiloBits/second", (8*total_bytes)/total_time/1000 );
     printf("\n * Total speed          = %f MegaBits/second", (8*total_bytes)/total_time/1000/1000 );
     printf("\n * Total speed          = %f GigaBits/second", (8*total_bytes)/total_time/1000/1000/1000 );
     printf("\n\n");

}


int main(int argc, char **argv)
{
     int portno;
     if (argc < 2) {
         fprintf(stderr,"usage: %s <portNumber>\n", argv[0]);
         exit(1);
     }
     portno = atoi(argv[1]);

	struct sockaddr_in myaddr;      /* our address */
        struct sockaddr_in remaddr;     /* remote address */
        socklen_t addrlen = sizeof(remaddr);            /* length of addresses */
        int recvlen;                    /* # bytes received */
        int fd;                         /* our socket */
        char *buffer = new char[CHUNK_SIZE];     /* receive buffer */

        /* create a UDP socket */

        if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("cannot create socket\n");
                return 0;
        }

        /* bind the socket to any valid IP address and a specific port */

        memset((char *)&myaddr, 0, sizeof(myaddr));
        myaddr.sin_family = AF_INET;
        myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        myaddr.sin_port = htons(portno);

        if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
                perror("bind failed");
                return 0;
        }
     uint64_t bytes;
     uint64_t total_bytes = 1;

     std::chrono::time_point<std::chrono::system_clock> start, end;
        if ((bytes=recvfrom(fd, buffer, CHUNK_SIZE, 0, (struct sockaddr *)&remaddr, &addrlen)) < 0 ) {
                printf("server error: errno %d\n",errno);
                perror("reading datagram");
                exit(1);
        }
                total_bytes = total_bytes + bytes;
     start = std::chrono::system_clock::now();
     while(total_bytes < (CHUNK_SIZE*NUM_LOOPS) ) {
        if ((bytes=recvfrom(fd, buffer, CHUNK_SIZE, 0, (struct sockaddr *)&remaddr, &addrlen)) < 0 ) {
                printf("server error: errno %d\n",errno);
                perror("reading datagram");
                exit(1);
        }
                total_bytes = total_bytes + bytes;
                if((total_bytes / CHUNK_SIZE) % 100000  == 0 ){
                  end = std::chrono::system_clock::now();
                  std::chrono::duration<double> elapsed_seconds = end-start;
                  double total_time = elapsed_seconds.count();
                  printer(total_bytes, total_time);
               }
     }
     end = std::chrono::system_clock::now();
     std::chrono::duration<double> elapsed_seconds = end-start;
     double total_time = elapsed_seconds.count();
     printer(total_bytes, total_time);
     close(fd);
     return 0;

}
