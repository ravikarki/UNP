/*
    Client code
*/
#include<stdio.h> //printf
#include<stdlib.h>
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<unistd.h>
#include<iostream>
#include<poll.h>

#define timeout 5000
#define retries 3 
#define STDIN 0
#define infinity -1

using namespace std;


/*Prints a error according to provided status as :
    1. Unknown command
    2. Incorrect Syntax
*/
void printError(int status, char comm[]=NULL)
{
    if(status == 1)
    {
        printf("CLIENT_ERROR \nInvalid Command \nCorrect Commands: GET/PUT/DEL/LOGOUT\n");
        return;
    }
    if(status == 2)
    {
        printf("CLIENT_ERROR \nToo few arguments \nCorrect Synatx is: %s \"table\" \"key\" ",comm);
    }
    if(status == 3)
    {
        printf("CLIENT_ERROR \nToo many arguments \nCorrect Synatx is: %s \"<table>\" \"<key>\" ",comm);
    }
    if(strcmp(comm,"PUT") == 0)
    {
        printf("<data>\n");
    }
    else
        printf("\n");

}
 
//Check for command error 
int checkformat(char client_message[])
{
    int k;
    int id_len, tab_len;
    id_len=tab_len=0;
    for(k=0; client_message[k]; k++)
    {
        if(client_message[k] >= 'a' && client_message[k] <='z')
            client_message[k] -= 32;      
    }
    if(strcmp(client_message,"LOG OUT") == 0)
        return 1;

    //printf("%s\n", client_message);
    char table[30]={'\0'};
    char command[4]={'\0'};
    char id[30]={'\0'};
    char data[30]={'\0'};
    int i,j;
    i=j=0;
    if(k<3)
    {
        printError(1);
        return -1;
    }
    for(i=0; i<3; i++, j++)
    {
        command[j] = client_message[i];       
    }

    
    command[j] = '\0';
    if(strcmp(command,"GET") != 0 && strcmp(command,"PUT") != 0 && strcmp(command,"DEL") != 0)
    {
        printError(1);
        return -1;
    }
    
    i+=1;       //For ' "'
    if(client_message[i] != '"')
    {
        printError(2,command);
        return -1;
    }
    i++;

    j=0;
    while(client_message[i] != '"')
    {
        if(i>=k)
        {
            printError(2,command);
            return -1;
        }        
        table[j] = client_message[i];
        i++;
        j++;
    }
    table[j] = '\0';
    tab_len = j+1;

    i+=2;       //For '" "'
    if(client_message[i] != '"')
    {
        printError(2,command);
        return -1;
    }
    i++;


    j=0;
    while(client_message[i] != '"')
    {
        if(i>=k)
        {
            printError(2,command);
            return -1;
        }
        id[j] = client_message[i];
        i++;
        j++;
    }
    id[j] = '\0';
    id_len = j+1;

    i++;
    if(client_message[i] && strcmp(command,"PUT") == 0)
    {
        i++;
        if(client_message[i] == '"')
        {
            i++;
            j=0;
            while(client_message[i] != '"')
            {
                if(i>=k)
                {
                    printError(2,command);
                    return -1;
                }
                data[j] = client_message[i];
                i++;
                j++;
            }
            data[j] = '\0';
        }   
        else
        {
            printError(2,command);
            return -1;
        }
    }
    else
    {
        if(client_message[i])
        {
            printError(3,command);
            return -1;
        }
        else if(strcmp(command,"PUT") == 0)
        {
            printError(2,command);
            return -1;   
        }
    }
    if(tab_len > 1 && id_len > 1)
        return 1;
    else
        printError(2,command);
    return -1; 
}


int main(int argc , char *argv[])
{
    if ( argc != 3 ) // argc should be 2 for correct execution
    {
    // We print argv[0] assuming it is the program name
        cout<<"usage: "<< argv[0] <<" <ip> <port>\n";
        return 0;
    }
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
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(argv[2]) );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected to Server\n");

    //Receive a reply from the server
    struct pollfd fds[2];   //for read and standard in
    fds[0].fd = sock;       //for socket read 
    fds[0].events = 0;  //Reset
    fds[0].events |= POLLIN;  

    fds[1].fd = STDIN;  //For standard input read
    fds[1].events = 0;  //Reset
    fds[1].events |= POLLIN;  
    
    int curr_retries=0;


    if( recv(sock , server_reply , 2000 , 0) < 0)
    {
        puts("recv failed");
        return 1;
    }

    puts(server_reply);
    bzero(server_reply, 2000);

    int pret = poll(fds, 2, infinity);

    //printf("\n%d",pret);
        
    while(fds[0].revents & POLLIN)
    {
        if( recv(sock , server_reply , 2000 , 0) <= 0)
        {
            puts("Server disconnected");
            return 1;
        }
        else
        {
            printf("Discarded a garbled/duplicate\n");
            pret = poll(fds, 2, infinity);
        }
    }

    cin.getline(message,1000);
        
    //Log in
    while(1)
    {
        
        //cin.ignore();
        
         
        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        pret = poll(fds, 1,timeout);
        
        if(pret == 0)
        {
            printf("Timeout occured! \n");
            curr_retries++;
            if(curr_retries >= retries)
            {
                printf("Maximum retries done. Server Unreachable \n Exiting...");
                return 1;
            }
            else
            {
                printf("Resending...\n");
                continue;
            }
        }

        else
        {
            curr_retries=0; 
            //Receive a reply from the server
            if( recv(sock , server_reply , 2000 , 0) < 0)
            {
                puts("Server disconnected");
                break;
            }   

            puts(server_reply);

            if(strcmp(server_reply,"Successfully Logged in.") == 0)
                break;
            bzero(server_reply, 2000);

            pret = poll(fds, 2,infinity);
        
            while(fds[0].revents & POLLIN)
            {
                if( recv(sock , server_reply , 2000 , 0) <= 0)
                {
                    puts("Server disconnected");
                    return 1;
                }
                else
                {
                    printf("Discarded a garbled/duplicate\n");
                    pret = poll(fds, 2, infinity);
                }
            }
 
            cin.getline(message,1000);        
        }
    }
    
    bzero(server_reply, 2000);


    bzero(message, 1000);

    pret = poll(fds, 2, infinity);
        
    while(fds[0].revents & POLLIN)
    {
        if( recv(sock , server_reply , 2000 , 0) <= 0)
        {
            puts("Server disconnected");
            return 1;
        }
        else
        {
            printf("Discarded a garbled/duplicate\n");
            pret = poll(fds, 2, infinity);
        }
    }

    cin.getline(message,1000);
     
    //keep communicating with server
    while(1)
    {
        while(checkformat(message) == -1)
        {
            bzero(message, 1000);
            pret = poll(fds, 2,timeout);        
            while(fds[0].revents & POLLIN)
            {
                if( recv(sock , server_reply , 2000 , 0) <= 0)
                {
                    puts("Server disconnected");
                    return 1;
                }
                else
                {
                    printf("Discarded a garbled/duplicate\n");
                    pret = poll(fds, 2, infinity);
                }
            }
            cin.getline(message,1000);
        }
        
        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        //puts("Send this failed");
         
        pret = poll(fds, 1,timeout);
        
        if(pret == 0)
        {
            printf("Timeout occured! \n");
            curr_retries++;
            if(curr_retries > retries)
            {
                printf("Maximum retries done. Server Unreachable \n Exiting...");
                return 1;
            }
            else
            {
                printf("Resending...\n");
                continue;
            }
        }

        else
        {
            curr_retries=0;         
        
            //Receive a reply from the server
            if( recv(sock , server_reply , 2000 , 0) < 0)
            {
                puts("Server disconnected");
                break;
            }   

            puts(server_reply);
            if(strcmp(server_reply,"Successfully logged out") == 0)
                    break;
            bzero(server_reply, 2000);
            bzero(message, 1000);

            pret = poll(fds, 1,timeout);

            while(fds[0].revents & POLLIN)
            {
                if( recv(sock , server_reply , 2000 , 0) <= 0)
                {
                    puts("Server disconnected");
                    return 1;
                }
                else
                {
                    printf("Discarded a garbled/duplicate\n");
                    pret = poll(fds, 2, infinity);
                }
            }
            cin.getline(message,1000);

        }
    }
     
    close(sock);
    return 0;
}