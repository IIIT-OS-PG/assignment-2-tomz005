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
#include <cstdlib>
#include <bits/stdc++.h>
#define BUFF_SIZE 512

using namespace std;
char* ipaddr= (char*)malloc(16);
int port;
map<string,bool>active_users;
map<string,bool>active_share;
map<string,pair<string,int>>client_ip_details;

bool writetofile(char username[],char password[])
{
    char u[50],p[50];
    //cout<<username<<":"<<password<<endl;
    FILE *users_fp=fopen("users.txt","a+");
    if(users_fp==NULL)
    {
        printf("[-] File couldnt be opened\n");
        exit(EXIT_FAILURE);
    }
    while(fscanf(users_fp,"%s %s",u,p)!=EOF)
    {
        //cout<<ipaddr;
        if(strcmp(u,username)==0)
        return 0;
    } 
    fprintf(users_fp,"%s ",username);
    fprintf(users_fp,"%s\n",password);
    fclose(users_fp);
    return 1;
}

bool check_user(char username[],char password[])
{
    char u[50],p[50];
    //cout<<username<<":"<<password<<endl;
    FILE *users_fp=fopen("users.txt","r");
    if(users_fp==NULL)
    {
        printf("[-] File couldnt be opened\n");
        exit(EXIT_FAILURE);
    }
    while(fscanf(users_fp,"%s %s",u,p)!=EOF)
    {
        //cout<<u<<p<<endl;
        if(strcmp(u,username)==0 && strcmp(p,password)==0)
        return 1;
    } 
    fclose(users_fp);
    return 0;
}

bool write_to_upload_file(char username[], char filepath[],char upload_filename[])
{
    //cout<<filepath<<peer_port<<endl;
    FILE* upload_fp=fopen("g1.txt","a+");
    if(upload_fp==NULL)
    {
        printf("[-] File couldnt be opened\n");
        return 0;
    }
    fprintf(upload_fp,"%s %s %s\n",upload_filename,filepath,username);
    fclose(upload_fp);
    return 1;
}

int check_in_upload(char filename[], char download_filepath[],char download_user[])
{
    //cout<<"in check_upload\n";
   //cout<<filename;
    FILE *upload_fp=fopen("g1.txt","r");
    char file_n[100],file_p[100],u[100];
    if(upload_fp==NULL)
    {
        printf("[-] File not presenr\n");
        return 0;
    }
    while(fscanf(upload_fp,"%s %s %s\n",file_n,file_p,u)!=EOF)
    {
        //cout<<file_n<<":"<<file_p<<":"<<u<<endl;
        if(strcmp(file_n,filename)==0)
        {
           // cout<<"match\n";
            //download_filepath=file_p;
            //download_user=u;
            strcpy(download_filepath,file_p);
            strcpy(download_user,u);
            //cout<<download_user;
        }
    }
    //cout<<"exit while loop\n";
    fclose(upload_fp);
    return 1;
}
void* client_handler(void* socket_desc)
{
    int c_socket = *(int *)socket_desc;
    struct sockaddr_in my_addr;
    char my_ip[16];
    int myPort;
    int my_addr_len=sizeof(my_addr);
    //printf("[+] In client thread of tracker\n");
    getsockname(c_socket, (struct sockaddr *) &my_addr, (socklen_t*)&my_addr_len);
	inet_ntop(AF_INET, &my_addr.sin_addr, my_ip, sizeof(my_ip));
	myPort = ntohs(my_addr.sin_port);
    //printf("[+] Client ip address: %s : %d\n", my_ip,myPort);
    char username[50],password[50],filepath[100],upload_filename[100],upload_username[50],peer_ip[16];
    int peer_port;
    char download_filename[100],download_filepath[100],download_user[50];
    int choice=0,br=0,upload_valid,download_valid=0;
    while(choice !=7)
    {   
        recv(c_socket, &choice, sizeof(int), 0);
        printf("\n[+] Choice : %d\n",choice);
        switch(choice)
            {
                case 1:
                recv(c_socket, &username, 50, 0);
                //printf("[+] Username : %s\n",username);
                recv(c_socket, &password, 50, 0);
                //printf("[+] Password : %s\n",password);
                int valid;
                if(writetofile(username,password))
                {
                    printf("User created and added to file\n");
                    valid=1;
                }
                else
                {
                    printf("User already exists\n");
                    valid=0;
                }
                send(c_socket,&valid,sizeof(valid),0);
                //recv(c_socket, &choice, sizeof(choice), 0);
                break;
                case 2:
                username[0]='\0';
                //cout<<sizeof(username);
                recv(c_socket,&username, sizeof(username), 0);
                //cout<<sizeof(username);
                //printf("[+] Username : %s\n",username);
                password[0]='\0';
                //cout<<sizeof(password);
                recv(c_socket,&password, sizeof(password), 0);
                //printf("[+] Password : %s \n",password);
                recv(c_socket,&peer_ip,sizeof(peer_ip),0);
                recv(c_socket,&peer_port,sizeof(peer_port),0);
                valid=check_user(username,password);
                if(valid)
                {
                    active_users.insert({username,true});
                    active_share.insert({username,true});
                    client_ip_details.insert({username,make_pair(peer_ip,peer_port)});

                }
               // cout<<active_users[username]<<" activelogin status\n";
                //cout<<active_share[username]<<" sharestaus\n";
                //cout<<client_ip_details[username].first<<": peer ip\n";
                //cout<<client_ip_details[username].second<<": peer port\n";
                send(c_socket,&valid,sizeof(valid),0);
                break;
                case 3:

                break;
                case 4:
                filepath[0]='\0';
                upload_filename[0]='\0';
                recv(c_socket,&upload_filename,sizeof(upload_filename),0);
                recv(c_socket, &filepath, sizeof(filepath), 0);
                cout<<filepath<<endl;
                //recv(c_socket,&upload_username,sizeof(upload_username),0);
                //cout<<peer_port<<endl;
                if(active_share[username])
                {
                    upload_valid=write_to_upload_file(username,filepath,upload_filename);
                    send(c_socket,&upload_valid,sizeof(upload_valid),0);
                }
                else
                {
                    upload_valid=2;
                    send(c_socket,&upload_valid,sizeof(upload_valid),0);
                }
                break;
                case 5:
                recv(c_socket,&download_filename,sizeof(download_filename),0);
                //cout<<download_filename;
                download_valid=check_in_upload(download_filename,download_filepath,download_user);
                //cout<<"back to main"<<download_valid<<endl;
                //cout<<download_filepath<<": "<<download_user;
                
                if(download_valid)
                {
                    //cout<<download_filepath<<"::"<<download_user<<endl;
                    if(active_share[download_user])
                    {
                        send(c_socket,&download_valid,sizeof(int),0);
                        //cout<<client_ip_details[download_user].first<<":"<<client_ip_details[download_user].second;
                        send(c_socket,download_filepath,sizeof(download_filepath),0);
                        send(c_socket,(client_ip_details[download_user].first).c_str(),16,0);
                        send(c_socket,&(client_ip_details[download_user].second),sizeof(int),0);
                    }
                    else
                    {
                        download_valid=0;
                        send(c_socket,&download_valid,sizeof(int),0);
                    }
                }
                else
                {
                    download_valid=2;
                    send(c_socket,&download_valid,sizeof(int),0);
                }
                
                //cout<<"[+] end of download loop\n";
                break;
                case 6:
                active_share[username]=false;
                break;
                case 7:
                active_users[username]=false;
                //if(!active_users[username])
                //cout<<"Disabled active status\n";
                break;
                default:
                break;
            }

    }
    close(c_socket);
    pthread_exit(0);

}

int readfile(const char* inpfile,int tracker_no)
{
    FILE *tracker_fp=fopen(inpfile,"r");
    
    int t;
    if(tracker_fp==NULL)
    {
        cout<<"error"<<endl;
        return 0; // no such file exists
    }
    
    while(fscanf(tracker_fp,"%d %s %d",&t,ipaddr,&port)!=EOF)
    {
        //cout<<ipaddr;
        if(tracker_no == t)
        return 1;
    }  
    return 2;
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
    printf("[+] Starting the tracker\n");
    //const char *tracker_info=argv[1];
    int tracker_no=atoi(argv[2]);
    int r=readfile(argv[1],tracker_no);
    if(r==1)
    printf("[+] Success reading ip & port for tracker\n");
    else
    {
        printf("[-] Failed reading ip & port\n");
        exit(EXIT_FAILURE);
    }
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       printf("[+] Server Socket created\n");
       
    //bzero(&server_address,sizeof(server_address));
    server_address.sin_family = AF_INET; 
    server_address.sin_addr.s_addr = inet_addr(ipaddr); 
    server_address.sin_port = htons(port); 
 
    // Forcefully attaching socket to the port 8080 
    int ret=bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address));
    if(ret<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    printf("[+] Port binding\n");
    printf("[+] Server ip :port %s:%d\n",inet_ntoa(server_address.sin_addr),ntohs(server_address.sin_port));
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
         if(client_socket<0)
        {
            cout<<"error in socket\n";
            exit(EXIT_FAILURE);
        }
        //printf("[+] Connection Accepted\n");
       // printf("[+] Connection accepted from %s:%d\n",inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port));
        if(pthread_create(&thread_id,NULL,client_handler,(void *)&client_socket)<0)
        {
            printf("[-] Thread creation failed\n");
            exit(EXIT_FAILURE);
        }
        
    }
    pthread_join(thread_id,NULL);
    close(client_socket);
    close(server_fd);
    return 0; 
}