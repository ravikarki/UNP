/*
    Client code
*/
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<unistd.h>
#include<iostream>

using namespace std;
 
//Check for command error 
int checkformat(char client_message[])
{
    char table[30]={'\0'};
    char command[4]={'\0'};
    char id[30]={'\0'};
    char data[30]={'\0'};
    int i,j;
    i=j=0;
    while(i=0; i<3; i++)
    {
        command[j] = client_message[i];
        i++;
        j++;
    }
  //  if(strcmp())
    command[j] = '\0';

    i+=2;       //For ' "'

    j=0;
    while(client_message[i] != '"')
    {
        if(client_message[i] >= 'a' && client_message[i] <='z')
            client_message[i] += 32;
        table[j] = client_message[i];
        i++;
        j++;
    }
    table[j] = '\0';

    i+=3;       //For '" "'

    j=0;
    while(client_message[i] != '"')
    {
        id[j] = client_message[i];
        i++;
        j++;
    }
    id[j] = '\0';

    i++;
    if(client_message[i])
    {
        i++;
        if(client_message[i] == '"')
        {
            i++;
            j=0;
            while(client_message[i] != '"')
            {
                data[j] = client_message[i];
                i++;
                j++;
            }
            data[j] = '\0';
        }   
    }
}


int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
     
    //Create IPv4 socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Server socket address 
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 9191 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected to Server\n");

    //Receive a reply from the server
    if( recv(sock , server_reply , 2000 , 0) < 0)
    {
        puts("recv failed");
    }

    puts("Server reply :");
    puts(server_reply);
    bzero(server_reply, 2000);

    
    //Log in
    while(1)
    {
        
        //cin.ignore();

        cin.getline(message,1000);
         
        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("Server disconnected");
            break;
        }   

        puts(server_reply);

        if(strcmp(server_reply,"Successfully Logged in.") == 0)
            break;
        
    }
    
    bzero(server_reply, 2000);


    //keep communicating with server
    while(1)
    {
        
        //cin.ignore();

        cin.getline(message,1000);
        if(checkformat(message) == -1)
            continue;
         
        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("Server disconnected");
            break;
        }   

        puts(server_reply);
        bzero(server_reply, 2000);

    }
     
    close(sock);
    return 0;
}