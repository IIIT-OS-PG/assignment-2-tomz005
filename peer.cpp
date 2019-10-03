// Peer needs to have both client & peer
// Only client mode as of now
#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <iostream>
#include <cstdlib>
#define PORT 8080
#define PORT1 9090
#define BUFF_SIZE 512
using namespace std;

//thread for handling client in server mode
void* request_handler(void* socket_desc)
{
    printf("In peer server client handler thread\n");
    int c_socket = *(int *)socket_desc;
    //printf("[+] Connection accepted from %s:%d\n",inet_ntoa(c_socket.sin_addr),ntohs(c_socket.sin_port));
    string file = "/home/tomz/Desktop/";
    string filename;
    char fn[50];
    //cout<<file<<endl;
    //cout<<c_socket<<endl;
    int ret=recv(c_socket, fn, sizeof(fn), 0);
    printf("[+] Received filename : %s\n",fn);
    filename=string(fn);
    cout<<filename<<endl;
    file.append(filename);
    cout<<file<<endl;
    FILE *fp = fopen(file.c_str(),"r");
    //int sum;
    //cout<<"Enter something";
    //cin>>sum;
    char Buffer[BUFF_SIZE];
    int file_size,n;
    if(fp==NULL)
    {
        printf("NULL");
        exit(0);
    }
    fseek(fp,0,SEEK_END);
    int size=ftell(fp);
    rewind(fp);
    //sending file size info to client peer
    //cout<<size<<endl;
    send(c_socket, &size,sizeof(size),0);
    while ( ( n = fread( Buffer , sizeof(char) , BUFF_SIZE , fp ) ) > 0  && size > 0 )
    {
        //cout<<"Sending\n";
        //cout<<n<<endl;
        //cout<<Buffer<<endl;
		send (c_socket , Buffer, n, 0 );
   	 	memset ( Buffer , '\0', BUFF_SIZE);
		size = size - n ;
    }

    printf("[+] File sent\n");
    close(c_socket);
    fclose(fp);
    pthread_exit(0);

}
//thread function for peer server mode
void* server_mode(void* port)
{
    int sport=*(int *)port;
    int peerserver_fd;
    int client_socket;
    struct sockaddr_in peerserver_address;
    struct sockaddr_in client_address; 
    socklen_t client_addr_size;
    int opt = 1; 
    int addrlen = sizeof(peerserver_address); 
    //char buffer[512] = {0}; 
    printf("[+] Starting the peer server mode\n");     
    // Creating socket file descriptor 
    if ((peerserver_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
    printf("[+] Peer-Server Socket created\n");
       
    //bzero(&server_address,sizeof(server_address));
    peerserver_address.sin_family = AF_INET; 
    peerserver_address.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    peerserver_address.sin_port = htons(sport); 
 
    // Forcefully attaching socket to the port 8080 
    int ret=bind(peerserver_fd, (struct sockaddr *)&peerserver_address, sizeof(peerserver_address));
    if(ret<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    printf("[+] Port binding\n");
    printf("[+] Peer-Server ip :port %s:%d\n",inet_ntoa(peerserver_address.sin_addr),ntohs(peerserver_address.sin_port));
    int status=listen(peerserver_fd,3);
    if (status < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 

   printf("[+] Server listening\n");
    //Multiple clients using pthread
    pthread_t thread_id;
    while(client_socket = accept(peerserver_fd, (struct sockaddr *)&client_address,(socklen_t*)&addrlen))
    {
        if(client_socket<0)
        {
            cout<<"error in socket\n";
            exit(EXIT_FAILURE);
        }
        printf("[+] Connection Accepted\n");
        printf("[+] Connection accepted from %s:%d\n",inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port));
        if(pthread_create(&thread_id,NULL,request_handler,(void *)&client_socket)<0)
        {
            printf("[-] Thread creation failed\n");
            exit(EXIT_FAILURE);
        }
        
    }
    pthread_join(thread_id,NULL);
    close(client_socket);
    close(peerserver_fd);
}


int main(int argc, char const *argv[]) 
{ 
    //Peer - Server mode thread
    pthread_t psid;
    FILE *fp1;
    int ch,port_for_servermode;
    port_for_servermode=atoi(argv[1]);
    printf("[+] Peer Server starting\n");
    pthread_create(&psid,NULL,server_mode,(void *)&port_for_servermode);
    //Remaining client mode
    //pthread_join(psid,NULL);
    sleep(2);
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    //char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   printf("[+]C: Socket creation done\n");
   //bzero(&serv_addr,sizeof(serv_addr));
   int psdport;
   cout<<"Enter peer-server port from where file to download\n";
   cin>>psdport;
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(psdport); 
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1"); 
   
    int status =connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(status<0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    printf("[+] Connection established with server\n");
    printf("[+] Connection established to %s:%d\n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));

//Menu for input from client
    do
    {
        cout<<"1.Create User\n";
        cout<<"2.Login\n";
        cout<<"3.List files\n";
        cout<<"4.Upload file\n";
        cout<<"5.Download file\n";
        cout<<"Enter choice\n";
        cin>>ch;
        switch(ch)
        {
            case 5:
             //Filename send
            char fname[50];
            cout<<"Enter file name\n";
            scanf("%s",fname);  
            fp1=fopen(fname,"wb");
            if(fp1==NULL)
            cout<<"NULL"<<endl;
            //cout<<"[+] File opened"<<endl;
            char Buffer1[BUFF_SIZE];
            send(sock, fname,sizeof(fname),0);
            //File download
            int file_size,n1;
            recv(sock, &file_size, sizeof(file_size), 0);
            //cout<<file_size<<endl;
            while ( ( n1 = recv( sock , Buffer1 ,   BUFF_SIZE, 0) ) > 0 )
            {
                //cout<<"receive\n";
                //cout<<n1<<endl;
                //cout<<Buffer1<<endl;
	            fwrite(Buffer1 , sizeof(char), n1, fp1);
                fflush(fp1);
	            memset( Buffer1 , '\0', BUFF_SIZE);
                //file_size = file_size - n1;
            } 
            printf("[+] File Downloaded\n");
            break;
            default:
            break;
        }

    }while(ch!=-1);

   
    fclose(fp1);
    //printf("[+] File sent\n");
    close(sock);
    return 0; 
} 