#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <poll.h>
#include <sys/wait.h>

#define SIZE 5000
#define TIMEOUT_SECS 2 

struct Ack_packet{
  uint16_t len ;
  uint32_t ackno;
}Ack_packet;

struct packet{
  uint16_t len;
  uint32_t seq;
  char data[SIZE];
};

double generateRand()
 { 
    return (double)rand() / (double)RAND_MAX ;
 }

void send_packets(struct packet Data[], int sock, struct sockaddr_in addr , int num ,int window_size , int start , double probFail){
        // sending cetrain window size 
        for(int i = start ; i  < (window_size+start); i++){
            if(i < num && probFail<generateRand()){
                printf("....packet # %d sent\n", i);
            if(sendto(sock, &Data[i], sizeof(struct packet), 0,(struct sockaddr*)&addr,sizeof(addr))==-1){
                 perror("ERROR in sending file.");
                 exit(1); 
            }
            }
        }
         printf("\n");
};
 void recv_ack(struct Ack_packet Ack, int num_of_packets , struct packet Data[], int sock, struct sockaddr_in addr  , int window_size ,int packet_num , double probFail){
     struct pollfd pfd = {.fd = sock, .events = POLLIN};
     socklen_t addr_size=sizeof(addr);
        int d = 0;
        int x;
        while(1){
          // here if no ack recieved within 1 second this mean time out and the packet loss so we resend it
          if(x = poll(&pfd, 1, 1000)!=0  ){
            recvfrom(sock, &Ack, sizeof(Ack), 0,(struct sockaddr*)&addr,&addr_size);
            printf("Ack # %d recived\n", Ack.ackno);
            if(Ack.ackno == num_of_packets-1){
               break;}
            packet_num  = packet_num +   window_size;
            window_size++;
            send_packets(Data, sock, addr, num_of_packets , window_size, packet_num, probFail);
              d++; 
            }else{
              printf("\n................Packet loss detected..............\n\n");
              window_size=1;
              packet_num= d ;
               send_packets(Data, sock, addr, num_of_packets , window_size, packet_num, probFail);
             Ack.ackno++;
            }   
    
         }
         printf("\n--------------Data sent----------------\n\n\n\n");

}

void send_file(FILE *fp ,int sock,struct sockaddr_in addr , double probFail){
     struct Ack_packet Ack ;
     int window_size = 1;
     int packet_num = 0;
     fseek(fp, 0, SEEK_END);
        int file_size =ftell(fp);
        double temp =(double)file_size/SIZE;
        int num_of_packets = (file_size/SIZE);
        if(temp>num_of_packets)
        {
          num_of_packets++;
        }
        fseek(fp, 0, SEEK_SET);
        char buffer[20];
        sprintf(buffer, "%d", file_size);
        // first we send the whole size of file to the client to predict to recieve length of packets 
        sendto(sock, buffer, sizeof(buffer), 0,(struct sockaddr*)&addr,sizeof(addr)); 
        struct packet  Data[num_of_packets] ; 

        // data setting stored as packets of struct array Data[]
        for(int j = 0 ; j < num_of_packets;j++){
            memset(Data[j].data, 0,SIZE);
            fread(Data[j].data, 1,SIZE, fp);
            Data[j].seq=j;
            Data[j].len=sizeof(Data[j].data);
        }
        //start sending the first packet
        send_packets(Data, sock, addr, num_of_packets , window_size, packet_num, probFail);

        //waiting for ack then sending more packets
         Ack.ackno = 0 ;
         recv_ack(Ack, num_of_packets, Data , sock , addr , window_size, packet_num,probFail);

}



int main(int argc , char *argv[]){
    int sock;
    struct sockaddr_in server;

    // create socket 
    sock =  socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == -1){
        printf("couldn't create socket");
    }
    puts("socket created");
    server.sin_addr.s_addr =inet_addr("127.0.0.1");
    server.sin_family =AF_INET;
    server.sin_port = htons(atoi(argv[1]));

     if(bind(sock,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    printf("Bind success...\n");
    int addr_size =sizeof(server);

    while(1){
      printf("\n......waiting for file path......\n\n\n");
      char fileName[20];
      recvfrom(sock,fileName,20,0,(struct sockaddr*)&server,&addr_size);
      FILE *file = fopen(fileName, "r");
        if(file ==NULL){
            perror("ERROR in reading file.\n");
            exit(1);
        }
        printf("\nFile path : %s\n\n",fileName);
        pid_t pid =fork();
        if(pid!=0)
        {
          sock =  socket(AF_INET, SOCK_DGRAM, 0); 
        }
        if(pid==0){
        srand(atoi(argv[2]));
        char *ptr;
        double probabFail =strtod(argv[3],&ptr);
        send_file(file,sock,server,probabFail);
        fclose(file);
        }

    }      
      close(sock); 
        return 0 ;

    }
