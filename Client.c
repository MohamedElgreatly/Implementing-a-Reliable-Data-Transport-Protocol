#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include <poll.h>
#define SIZE 5000


// Ack packet data structure
struct Ack_packet{
  uint16_t len ;
  uint32_t ackno;
}Ack_packet;

// packet data structure
struct packet{
  uint16_t len ;
  uint32_t seqno;
  char data[SIZE];
}packet;


// write data into file function 
void write_file(FILE *fp,int sockfd ,struct sockaddr_in addr, socklen_t addr_size,  int sizFile){
  struct packet Data ; // making data structure of type packet to hold the packet recived data
  struct Ack_packet Ack; //making data structure of type Ack  packet to hold the packet recived ack data
  memset(Data.data, 0, SIZE); // setting all data array bytes to zero
  Ack.ackno = 0 ;//set the ack with dummy data 0
  int ack_seq = 0 ;
  // main loop to get the data from the server
  while (Ack.ackno  < (sizFile/SIZE) )
  { 
    recvfrom(sockfd,&Data,sizeof(Data),0,(struct sockaddr*)&addr,&addr_size);  //recive packet from the clietn
   // check if the recived data sequence number is correct
     if (Data.seqno == ack_seq){
        ack_seq++;    // increament the ack sequence number
       Ack.ackno=Data.seqno; // set the current ack number with the packet sequence number
          printf("\n.....sequence # %d \n", Data.seqno); // print the sequence number
         printf("........ sending ack # %d \n", Ack.ackno); //  print the ack number
         sendto(sockfd,&Ack,64,0,(struct sockaddr*)&addr,sizeof(addr)); //seend the ack message to the server
         fwrite(Data.data, 1, SIZE, fp); // write the packet data to the file
         memset(Data.data, 0, SIZE); // set the data filed with zeros
       }
  }
  printf("\n--------------Data recived----------------\n\n\n\n");
  return;

}


// main function
int main(int argc , char *argv[])
{
    int sockfd; // variable to hold the socket
    struct sockaddr_in  client; // variable to hold the socket address

    //Create socket
    sockfd = socket(AF_INET , SOCK_DGRAM ,0);
    // check if the socket was created 
    if (sockfd == -1)
    {
  
        printf("Could not create socket");
    }
    // socket created message
    puts("Socket created");

    //Prepare the sockaddr_in structure
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_port = htons(atoi(argv[1]));
    //Bind
   char fileName[20]; // variable to hold the file name
   char newfileName[20]; // variable to hold the new file name
   ///main loop to continue sending new files
    while(1){
       printf("\nPlease enter file path: ");
       scanf("%s",fileName);// getting the file name
       printf("\nPlease enter new  file path : ");
       scanf("%s",newfileName);// getting the new file name
       printf("\n\n"); 
       FILE *fp=fopen(newfileName,"w"); // creating new file 
         socklen_t addr_size; // variable to hold address size
         char sizeBuff[20];// char array to get the all file size
         // check if the the file hasn't been created
        if(fp ==NULL)
        {
          perror("ERROR in creating file.");
          exit(1);
        }
         struct pollfd pfd = {.fd = sockfd, .events = POLLIN};
        while(1){
        sendto(sockfd, fileName,20, 0,(struct sockaddr*)&client,sizeof(client)); //sending the file path
        if( poll(&pfd, 1, 1000)!=0  ){
        //recive the file size from the user
        recvfrom(sockfd,sizeBuff,20,0,(struct sockaddr*)&client,&addr_size);
        break;
        }else{
          printf("file name packet lost");
        }
        }
       // calling the write file function
      int sizFile =atoi(sizeBuff); // assigning the recived file size
       addr_size=sizeof(client);   
       write_file(fp,sockfd,client,addr_size, sizFile );
       fclose(fp); //close the file
      }
      // ending all transactions close the socket
    close(sockfd);
    puts("Handler assigned");

    
    return 0;
}