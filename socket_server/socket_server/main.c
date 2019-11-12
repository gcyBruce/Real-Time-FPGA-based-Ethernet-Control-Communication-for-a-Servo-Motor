/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

// http://www.linuxhowtos.org/C_C++/socket.htm
// http://www.linuxhowtos.org/data/6/server.c

// function: Create a socket server. Dump the received message from the client and loopack the message to client.
// uage: ./socket_server port_no (porn_no is a interger larger than 2000.)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "hps_0.h"

#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     int angle;
     int speed;

     void *virtual_base;
	 int fd;
	 int loop_count;
	 int led_direction;
	 int led_mask;
	 int gpio_mask;
	 int *h2p_lw_led_addr;

 	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
	printf( "ERROR: could not open \"/dev/mem\"...\n" );
	return( 1 );
	}

	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );

	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}
	
	h2p_lw_led_addr=virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + LED_PIO_BASE ) & ( unsigned long)( HW_REGS_MASK ) );

          if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
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
     float cur = 0; //initialize current angle
     while(1)
     {
    
     //input angle
     n = write(newsockfd,"Please input angle:",19);
     if (n < 0) error("ERROR writing to socket angle");
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     angle = atoi(buffer);
    
    
     //input speed
     n = write(newsockfd,"Please input speed:",19);
     if (n < 0) error("ERROR writing to socket speed");
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     speed = atoi(buffer);

     printf("Target angle: %d Target speed: %d \n", angle, speed);
     

	 float degree_max = 2.5;
	 float degree_min = 0.5;
	 int min_angle = 0;
	 int max_angle = 180;
	 float high_dur;

/*
     printf("input speed\n");
     scanf("%d\n",&speed);
     printf("input angle");
     scanf("%d\n",&angle);
     printf("speed: %d  angle:%d\n",speed,angle);
     //speed = 0.2;
     //angle = 175;
*/
	 

	 
	 float res;
	 if (cur < angle){
	     while (cur < angle){
            printf("cur%f\n",cur);
		 	cur += speed;
		 	*h2p_lw_led_addr = 0x1f;
		 	high_dur = (degree_max-degree_min) / 180 * cur + degree_min;
		 	usleep(high_dur * 1000);
		 	*h2p_lw_led_addr = 0;
		 	res = 20-high_dur;
		 	usleep(res * 1000);

	    	}
	 	}
	 else{
	     while (cur > angle){
		 	cur -= speed;
		 	*h2p_lw_led_addr = 0x1f;
		 	high_dur = (degree_max-degree_min) / 180 * cur + degree_min;
		 	usleep(high_dur * 1000);
		 	*h2p_lw_led_addr = 0;
		 	res = 20-high_dur;
		 	usleep(res * 1000);

	     	}
	 	 }

 	 }
     close(newsockfd);
     close(sockfd);
     return 0; 
}