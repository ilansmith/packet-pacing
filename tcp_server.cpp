/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <chrono>
#include <iostream>
#include <ctime>


#include "buff_size.h"


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

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


int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[ CHUNK_SIZE ];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"usage: %s <portNumber>\n", argv[0]);
         exit(1);
     }
     portno = atoi(argv[1]);

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");
     bzero(buffer, CHUNK_SIZE);
     printf("\n client connected!\n");
    std::chrono::time_point<std::chrono::system_clock> start, end;
     n = read(newsockfd,buffer,1);
     start = std::chrono::system_clock::now();
     if (n < 0) error("ERROR reading from socket");
     uint64_t bytes;
     long total_bytes = 1;
     int i;
     while(total_bytes < (CHUNK_SIZE*NUM_LOOPS) ) {
                bytes = read(newsockfd,buffer, CHUNK_SIZE);
                if(bytes < 0 ) {
                  perror("cannot read from socket");
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
     printf("\n * Total bytes read     = %f ", total_bytes);
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
     close(newsockfd);
     close(sockfd);
     return 0; 
}


