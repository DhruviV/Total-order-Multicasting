

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>


#define HELLO_PORT 12345
#define HELLO_GROUP "225.0.0.37"
#define MSGBUFSIZE 256
char initialSetup()
{
    char message;
    printf("Please select a unique name for this node between characters A to Z (Except C, it is reserved)\n");
    scanf("%c",&message);
    return message;
}

main(int argc, char *argv[])
{
     struct sockaddr_in addr;
     int fd, nbytes,addrlen;
     struct ip_mreq mreq;
     char msgbuf[MSGBUFSIZE];
     struct sockaddr_in addr1;
     char flag;
     int sendSocket, cnt1,clock;
     time_t t;
     struct ip_mreq mreq1;
     char message[]="Hey I am listener!";


     struct packet{
        int clock;
        char message;
        int updateFlag;
     };


     struct packet listeningPacket;
     struct packet sendingPacket;
     sendingPacket.message = initialSetup();
     sendingPacket.updateFlag = 0;
     u_int yes=1;            

 
     srand((unsigned) time(&t));
     clock=rand()%10;
     printf("This is my clock: %d\n", clock);
     sleep(1);
     sendingPacket.clock = clock; 

     if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
	  perror("socket");
	  exit(1);
     }

    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
       perror("Reusing ADDR failed");
       exit(1);
       }

     memset(&addr,0,sizeof(addr));
     addr.sin_family=AF_INET;
     addr.sin_addr.s_addr=htonl(INADDR_ANY); 
     addr.sin_port=htons(HELLO_PORT);

     if ((sendSocket=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
	  perror("socket");
	  exit(1);
     }

     memset(&addr1,0,sizeof(addr1));
     addr1.sin_family=AF_INET;
     addr1.sin_addr.s_addr=inet_addr(HELLO_GROUP);
     addr1.sin_port=htons(HELLO_PORT);

     if (bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	  perror("bind");
	  exit(1);
     }

     mreq.imr_multiaddr.s_addr=inet_addr(HELLO_GROUP);
     mreq.imr_interface.s_addr=htonl(INADDR_ANY);
     if (setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
	  perror("setsockopt");
	  exit(1);
     }

     int counter =0;

     while (1) {
	  while(counter<10){
          addrlen=sizeof(addr);

          if ((nbytes=recvfrom(fd,&listeningPacket,sizeof(struct packet),0,(struct sockaddr *) &addr,&addrlen)) < 0) {
               perror("recvfrom");
               exit(1);
          }


          clock++;
        
          counter++;

          sleep(1);

          if(listeningPacket.message=='C' && listeningPacket.updateFlag==0){
            flag = 's';
            break;
          }

          if(listeningPacket.message=='C' && listeningPacket.updateFlag==1){
            flag = 's';
            printf("My clock has been updated to : %d\n",listeningPacket.clock);
            return 0;
          }
      }
      counter = 0;
      printf("Hi I am sending..\n");
      int sentCounter=0;

      while(sentCounter<2){

          if (sendto(sendSocket,&sendingPacket,sizeof(struct packet),0,(struct sockaddr *) &addr1,sizeof(addr1)) < 0) {
               perror("sendto");
               exit(1);
          }
          clock++;
          sentCounter++;

          sleep(1);
      }

    }
}
