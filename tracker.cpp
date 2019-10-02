// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <cstring>
#include <iostream> 
#include <arpa/inet.h> 
#include <pthread.h>
#define PORT 8080 
#define BUFF_SIZE 512

using namespace std;

//char ch='a';
void* connection_handler(void* socket_desc)
{
    int c_socket = *(int *)socket_desc;
    //printf("[+] Connection accepted from %s:%d\n",inet_ntoa(c_socket.sin_addr),ntohs(c_socket.sin_port));
    string file = "/home/tomz/Desktop/";
    string filename;
    char fn[50];
    cout<<file<<endl;
    recv(c_socket, fn, sizeof(fn), 0);
    //printf("[+] Received filename : %s\n",filename);
    //filename=string(fn);
    //fn =*(char *)filename;
    filename=string(fn);
    cout<<filename<<endl;
    file.append(filename);
    cout<<file<<endl;
    FILE *fp = fopen(file.c_str(),"rb");
    int sum;
    cout<<"Enter something";
    cin>>sum;
    char Buffer[BUFF_SIZE];
    int file_size,n;
    /*recv(c_socket, &file_size, sizeof(file_size), 0);

    while ( ( n = recv( c_socket , Buffer ,   BUFF_SIZE, 0) ) > 0  &&  file_size > 0)
    {
	    fwrite (Buffer , sizeof (char), n, fp);
	    memset ( Buffer , '\0', BUFF_SIZE);
        file_size = file_size - n;
    } 


    printf("File written\n");*/

    //Sending file
        //sending file size info to server
    if(fp==NULL)
    {
        printf("NULL");
        exit(0);
    }
    fseek(fp,0,SEEK_END);
    int size=ftell(fp);
    rewind(fp);
    //sending file size info to server
    cout<<size<<endl;
    send(c_socket, &size,sizeof(size),0);
    while ( ( n = fread( Buffer , sizeof(char) , BUFF_SIZE , fp ) ) > 0  &&size > 0 )
    {
		send (c_socket , Buffer, n, 0 );
   	 	memset ( Buffer , '\0', BUFF_SIZE);
		size = size - n ;
    }

    printf("[+] File sent\n");
    close(c_socket);
    fclose(fp);
    pthread_exit(0);

}


int main(int argc, char const *argv[]) 
{ 
    int server_fd, client_socket, valread; 
    struct sockaddr_in server_address;
    struct sockaddr_in client_address; 
    socklen_t client_addr_size;
    int opt = 1; 
    int addrlen = sizeof(server_address); 
    char buffer[512] = {0}; 
    //char *hello = "Hello from server"; 
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       printf("[+] Server Socket created\n");
       
    //bzero(&server_address,sizeof(server_address));
    server_address.sin_family = AF_INET; 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    server_address.sin_port = htons( PORT ); 
 
    // Forcefully attaching socket to the port 8080 
    int ret=bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address));
    if(ret<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    printf("[+] Port binding\n");
    printf("Server ip :port %s:%d\n",inet_ntoa(server_address.sin_addr),ntohs(server_address.sin_port));
    int status=listen(server_fd,3);
    if (status < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 

   printf("[+] Server listening\n");
    //Multiple clients using pthread
    pthread_t thread_id;
    while(client_socket = accept(server_fd, (struct sockaddr *)&client_address, &(client_addr_size)))
    {
        printf("[+] Connection Accepted\n");
        printf("[+] Connection accepted from %s:%d\n",inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port));
        if(pthread_create(&thread_id,NULL,connection_handler,(void *)&client_socket)<0)
        {
            printf("[-] Thread creation failed\n");
            exit(EXIT_FAILURE);
        }
        
    }
    pthread_join(thread_id,NULL);
   /*Single slient server handling
    client_socket = accept(server_fd, (struct sockaddr *)&client_address, &(client_addr_size));
    if(client_socket<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 
    printf("[+] Connection accepted from %s:%d\n",inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port));
    char *file = "/home/tomz/Desktop/sa";
    FILE *fp = fopen(file,"wb");
    char Buffer[BUFF_SIZE];
    int file_size,n;
    recv(client_socket, &file_size, sizeof(file_size), 0);

    while ( ( n = recv( client_socket , Buffer ,   BUFF_SIZE, 0) ) > 0  &&  file_size > 0)
    {
	    fwrite (Buffer , sizeof (char), n, fp);
	    memset ( Buffer , '\0', BUFF_SIZE);
        file_size = file_size - n;
    } 




    fclose(fp);*/

    /*string read and send
    valread = read( client_socket , buffer, 1024); 
    printf("%s\n",buffer ); 
    send(client_socket , hello , strlen(hello) , 0 ); 
    printf("[+] Hello message sent\n"); */
    close(client_socket);
    close(server_fd);
    return 0; 
}