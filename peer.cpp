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
#include <sys/stat.h>
//#include <openssl/sha.h>
#define PORT 8080
#define PORT1 9090
#define BUFF_SIZE 512
using namespace std;

char* tracker_ipaddr= (char*)malloc(16);
int tracker_port;
bool login;


//Read tracker_info.txt
int readfile(const char* inpfile,int tracker_no)
{
    FILE *tracker_fp=fopen(inpfile,"r");
    
    int t;
    if(tracker_fp==NULL)
    {
        cout<<"error"<<endl;
        return 0; // no such file exists
    }
    
    while(fscanf(tracker_fp,"%d %s %d",&t,tracker_ipaddr,&tracker_port)!=EOF)
    {
        //cout<<ipaddr;
        if(tracker_no == t)
        return 1;
    }  
    return 2;
}



//thread for handling client in server mode
void* request_handler(void* socket_desc)
{
    printf("In peer server client handler thread\n");
    int c_socket = *(int *)socket_desc;
    //printf("[+] Connection accepted from %s:%d\n",inet_ntoa(c_socket.sin_addr),ntohs(c_socket.sin_port));
    //string file = "/home/tomz/Desktop/";
    string filename;
    char fn[50];
    //cout<<file<<endl;
    //cout<<c_socket<<endl;
    int ret=recv(c_socket, fn, sizeof(fn), 0);
    printf("[+] Received filepath : %s\n",fn);
    filename=string(fn);
    cout<<filename<<endl;
    //file.append(filename);
    //cout<<file<<endl;
    FILE *fp = fopen(filename.c_str(),"r");
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
        printf("[+]P2PS: Connection Accepted\n");
        printf("[+]P2PS: Connection accepted from %s:%d\n",inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port));
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

int download_from_peer(char filename[],char filepath[],char peerip[],int peerport)
{
    struct sockaddr_in peer_server_address;
    int peer_server_address_len=sizeof(peer_server_address);
    int peer_sock;
    FILE *fp1;
    if ((peer_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   printf("[+]P2PC: Socket creation done\n");
    peer_server_address.sin_family = AF_INET; 
    peer_server_address.sin_addr.s_addr = inet_addr(peerip); 
    peer_server_address.sin_port = htons(peerport); 
    int status =connect(peer_sock, (struct sockaddr *)&peer_server_address,peer_server_address_len);
    if(status<0)
    {
        printf("[-] Connection failed with peer server\n");
        return -1;
    }
    printf("[+]P2PC: Connection established to %s:%d\n",inet_ntoa(peer_server_address.sin_addr),ntohs(peer_server_address.sin_port));
    send(peer_sock,filepath,100,0);
    fp1=fopen(filename,"wb");
    if(fp1==NULL)
    cout<<"NULL"<<endl;
            //cout<<"[+] File opened"<<endl;
    char Buffer1[BUFF_SIZE];
            //File download
    int file_size,n1;
    recv(peer_sock, &file_size, sizeof(file_size), 0);
    cout<<file_size<<endl;
    while ( ( n1 = recv(peer_sock , Buffer1 ,   BUFF_SIZE, 0) ) > 0 )
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

}

int main(int argc, char const *argv[]) 
{ 
    //own ip
    struct sockaddr_in my_addr;
    char my_ip[16];
    int my_addr_len=sizeof(my_addr);
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
    struct sockaddr_in tracker_addr; 
    //char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   printf("[+]C: Socket creation done\n");
   //bzero(&serv_addr,sizeof(serv_addr));
    //Calling readfile to get tracker_1 ip & port
    int r=readfile(argv[2],1);
    if(r==1)
    printf("[+] Success reading ip & port for tracker\n");
    else
    {
        printf("[-] Failed reading ip & port\n");
        exit(EXIT_FAILURE);
    }

    tracker_addr.sin_family = AF_INET; 
    tracker_addr.sin_addr.s_addr = inet_addr(tracker_ipaddr); 
    tracker_addr.sin_port = htons(tracker_port); 

/*   int psdport;
   cout<<"Enter tracker port \n";
   cin>>psdport;
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(psdport); 
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1"); 
*/
   
    int status =connect(sock, (struct sockaddr *)&tracker_addr, sizeof(tracker_addr));
    if(status<0) 
    { 
        printf("\nConnection Failed with tracker 1 \n");
         //Connect with tracker 2
         r=readfile(argv[2],2);
         tracker_addr.sin_addr.s_addr = inet_addr(tracker_ipaddr); 
         tracker_addr.sin_port = htons(tracker_port); 
         status=connect(sock, (struct sockaddr *)&tracker_addr, sizeof(tracker_addr));
         if(status<0)
         {
             printf("\nConnection Failed with tracker 2 also. Check if they are up\n");
             return -1;
         }
    } 
    printf("[+] Connection established with tracker\n");
    printf("[+] Connection established to %s:%d\n",inet_ntoa(tracker_addr.sin_addr),ntohs(tracker_addr.sin_port));

    getsockname(sock, (struct sockaddr *) &my_addr, (socklen_t*)&my_addr_len);
	inet_ntop(AF_INET, &my_addr.sin_addr, my_ip, sizeof(my_ip));
	int myPort = ntohs(my_addr.sin_port);
    printf("Client ip address: %s : %d\n", my_ip,myPort);
//Login & create account has to be before he can opt for functionalities
char username[50],password[50];
int choice,login_valid,upload_valid;
do
{
    cout<<"1.Create User\n";
    cout<<"2.Login\n";
    cout<<"Enter choice\n";
    cin>>ch;
    switch(ch)
    {
        case 1:
        //strcpy(choice,"Create");
            choice=1;
            username[0]='\0';
            send(sock,&choice,sizeof(int),0);
            cout<<"Enter username:\n";
            cin>>username;
            send(sock,username,50,0);
            password[0]='\0';
            cout<<"Enter password:\n";
            cin>>password;
            send(sock,password,50,0);
            int valid;
            recv(sock,&valid,sizeof(valid),0);
            if(valid)
            cout<<"[+] Account created\n";
            else
            cout<<"[-] Account exists\n";
            break;
        case 2:
            choice=2;
            username[0]='\0';
            password[0]='\0';
            send(sock,&choice,sizeof(int),0);
            cout<<"Enter username:\n";
            cin>>username;
            send(sock,username,50,0);
            
            cout<<"Enter password:\n";
            cin>>password;
            send(sock,password,50,0);
            send(sock,my_ip,16,0);
            send(sock,&port_for_servermode,sizeof(int),0);
            recv(sock,&login_valid,sizeof(login_valid),0);
            if(login_valid)
            cout<<"[+] Login successful\n";
            else
            cout<<"[-] Invalid username/password\n";
            break;
        default:
        break;
    }

}while(login_valid!=1);

char filepath[100],upload_filename[100];

//Menu for input from client
    do
    {
        cout<<"3.List files\n";
        cout<<"4.Upload file\n";
        cout<<"5.Download file\n";
        cout<<"6.List files\n";
        cout<<"7.Stop Sharing\n";
        cout<<"8.Logout\n";
        cout<<"Enter choice\n";
        cin>>ch;
        switch(ch)
        {
            case 4:
            choice=4;
            send(sock,&choice,sizeof(int),0);
            filepath[0]='\0';
            upload_filename[0]='\0';
            cout<<"Enter filename\n";
            cin>>upload_filename;
            cout<<"Enter file path\n";
            cin>>filepath;

            // need to calculate sha//

            if(access(filepath, F_OK) != -1)
            {
                cout<<"File exists\n";
                send(sock,upload_filename,strlen(upload_filename),0);
                send(sock,filepath,strlen(filepath),0);
                recv(sock,&upload_valid,sizeof(upload_valid),0);
                if(upload_valid==1)
                cout<<"[+] Upload successful\n";
                else if(upload_valid==0)
                cout<<"[-] Upload failed\n";
                else
                cout<<"[-] Sharing disabled\n";
            }
            else
            {
                cout<<"File doesnt exist\n";
            }
            
            break;
            case 5:
             //Filename send
            choice=5;
            send(sock,&choice,sizeof(int),0);
            char fname[50],download_peer_ip[16],download_path[100];
            int download_peer_port;
            cout<<"Enter file name\n";
            scanf("%s",fname);  

            send(sock, fname,sizeof(fname),0);
            recv(sock, &download_path,100,0);
            recv(sock, &download_peer_ip,16,0);
            recv(sock, &download_peer_port,sizeof(int),0);
            printf("[+] Destination path : %s\n",download_path);
            printf("[+] Destination ip : %s:%d\n",download_peer_ip,download_peer_port);
            download_from_peer(fname,download_path,download_peer_ip,download_peer_port);
            /*fp1=fopen(fname,"wb");
            if(fp1==NULL)
            cout<<"NULL"<<endl;
            //cout<<"[+] File opened"<<endl;
            char Buffer1[BUFF_SIZE];*/
            //File download
            /*int file_size,n1;
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
            printf("[+] File Downloaded\n");*/
            break;
            default:
            break;
        }

    }while(ch!=8);

   
    //fclose(fp1);
    //printf("[+] File sent\n");
    close(sock);
    return 0; 
} 