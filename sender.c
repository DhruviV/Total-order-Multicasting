
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

int initialSetup()
{
    int x;
    printf("Please enter the number of listener nodes you want to create\n");
    scanf("%d",&x);
    printf("You have decided to create : %d nodes\n",x);
    return x;
}

int setTime(int array[], char arrayMsg[], int counter, int localClock){

    int averageClock =0;
    for(int i=0; i<counter;i++)
    {
        printf("The values in the array are: %d\n",array[i]);
    }
    for(int i=0; i<counter;i++)
    {
        averageClock = averageClock + array[i];
    }
    averageClock = averageClock/counter;
    return averageClock;
}

main(int argc, char *argv[])
{
     struct sockaddr_in addr;
     int fd, nbytes,addrlen;
     struct ip_mreq mreq;
     char msgbuf[MSGBUFSIZE];
     struct sockaddr_in addr1;
     char flag;
     int sendSocket, cnt1, clock;
     struct ip_mreq mreq1;
     char message[]="";
     char arrayMsg[10]="";
     int x = initialSetup(); 
     time_t t;
     int array[1000];


     u_int yes=1;            

     struct packet{
        int clock;
        char message;
        int updateFlag; 
     };
     struct packet listeningPacket;
     struct packet sendingPacket;

     sendingPacket.message = 'C';
     sendingPacket.updateFlag =0;

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
     int sentCounter=0;
     int alreadyExists;
     int loopCounter =0;
     flag = 's';
     int averageClock;
     struct packet syncClocksPacket;
     int exitFlag=0;

     while (1) {
      while(sentCounter<x){
          if(flag=='a')
          {
            averageClock = setTime(array,arrayMsg,counter, clock);
            printf("The value of averageClock is : %d\n", averageClock);
            sendingPacket.clock = averageClock;
            sendingPacket.message = 'C';
            sendingPacket.updateFlag=1;
            flag = 's';
            exitFlag = 1;
          }

          if (sendto(sendSocket,&sendingPacket,sizeof(struct packet),0,(struct sockaddr *) &addr1,sizeof(addr1)) < 0) {
               perror("sendto");
               exit(1);
          }
          clock++;
          sentCounter++;
          sleep(1);

          if(exitFlag==1)
          {
            return 0;
          }
      }
      sentCounter=0;
      sleep(1);
	  flag = 'r';
      printf("Sender is now listening to nodes.\n");

	  while(counter<x){
          alreadyExists = 0;
          addrlen=sizeof(addr);
          if ((nbytes=recvfrom(fd,&listeningPacket,sizeof(struct packet),0,(struct sockaddr *) &addr,&addrlen)) < 0) {
               perror("recvfrom");
               exit(1);
          }
          clock++;
          sleep(1);

              for(int k =0;k<x;k++)
              {
                if(arrayMsg[k]==listeningPacket.message || listeningPacket.message=='C')
                {

                    alreadyExists = 1 ;
                    break;
                }
              }

              if(alreadyExists!=1){
                arrayMsg[counter]=listeningPacket.message;
                array[counter] = listeningPacket.clock;
                counter++;

              }


            if(counter == x)
            {
                flag='a';
                break;
            }
            loopCounter++;
            flag ='s';
      }

    }
}
