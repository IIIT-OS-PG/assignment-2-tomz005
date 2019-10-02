// Peer needs to have both client & peer
// Only client mode as of now
#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <iostream>
#define PORT 8080 
#define BUFF_SIZE 512

using namespace std;
int main(int argc, char const *argv[]) 
{ 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    //char *hello = "Hello from client"; 
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   printf("[+] Socket creation done\n");
   bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1"); 
    // Convert IPv4 and IPv6 addresses from text to binary form 
    // if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    // { 
    //     printf("\nInvalid address/ Address not supported \n"); 
    //     return -1; 
    // } 
   
    int status =connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(status<0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    printf("[+] Connection established with server\n");
    //Filename send
    char fname[50];
    scanf("%s",fname);  
    FILE *fp=fopen(fname,"wb");
    if(fp==NULL)
    cout<<"NULL"<<endl;
    cout<<"[+] File opened"<<endl;
    char Buffer[BUFF_SIZE];
    send(sock, fname,sizeof(fname),0);
    //File download
    int file_size,n;
    recv(sock, &file_size, sizeof(file_size), 0);

    while ( ( n = recv( sock , Buffer ,   BUFF_SIZE, 0) ) > 0  &&  file_size > 0)
    {
	    fwrite (Buffer , sizeof (char), n, fp);
	    memset ( Buffer , '\0', BUFF_SIZE);
        file_size = file_size - n;
    } 


    printf("[+] File Downloaded\n");
    fclose(fp);
    //printf("[+] File sent\n");
    /*String send
    send(sock , hello , strlen(hello) , 0 ); 
    printf("[+] Hello message sent\n"); 
    valread = read( sock , buffer, 1024); 
    printf("%s\n",buffer ); */
    close(sock);
    return 0; 
} 