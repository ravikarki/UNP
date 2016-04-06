//Server 

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include<mysql.h>
#include<my_global.h>

//using namespace std;

//The client handler function
void *client_service(void *);

int main(int argc, char *argv[])
{
	//Listening socket descriptor, Client socket descriptor
	int listenfd, connfd;

	//Server Socket Address, Connecting client
	struct sockaddr_in servaddr, clientaddr;

	/*Create Server Socket
	int socket(family, type, protocol)
	AF_INET : IPv4 family
	SOCK_STREAM : Stream Socket
	0 : TCP Transport Protocol
	Returns non negative descriptor if OK
	-1 if Error
	*/

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	//Check for error
	if(listenfd == -1)
	{
		printf("\nError in creating Server Socket. Exiting...");
		exit(1);
	}
	printf("\nSocket created");


	bzero(&servaddr, sizeof(servaddr));

	//Create server structure
	servaddr.sin_family = AF_INET;	//Server family is IPv4

	//htonl() = Converts host byte order to network byte order
	//INADDR_ANY = Wildcard, will be chosen by OS
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//Assign a port to socket
	servaddr.sin_port = htons(9191);

	/*Bind the Socket
	int bind(int sockfd, const struct sockaddr *myaddr, socklength)
	Assigns a local protocol address to the socket
	*/
	if( bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
		//Error occurred
		perror("\nError occurred in binding. Exiting...");
		exit(1);
	}
	printf("\nBind done");

	/*Listen for incoming connections
	int listen(int sockfd, int backlog)
		*/

	if(listen(listenfd, 3))
	{
		//Error Occurred
		perror("\n Error occurred in Listening. Exiting...");
		exit(1);
	}
	puts("\n Server started...");

	socklen_t len=sizeof(clientaddr);

	while( (connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &len) ) )
    {
        puts("Connection accepted");
        pthread_t servicethread;

        /*
        Each client is handled by a separate thread
        int pthread_create(pthread_t *thread, pthread_attr_t *attr, 
                   void *(*start_routine)(void *), void *arg);

        Here 
        pthread_t *thread: the actual thread object that contains pthread id
		pthread_attr_t *attr: attributes to apply to this thread
		void *(*start_routine)(void *): the function this thread executes
		void *arg: arguments to pass to thread function above
        */
        if( pthread_create( &servicethread , NULL, client_service, (void *)&connfd) < 0 )
        {
        	perror("\n Could not create service threadd");
        	exit(1);
        } 
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("\n Handler assigned");
    }

    perror("\n Accept failed");

    return 0;

}


//Validates if client's username and password are correct
//Returns 1 if valid
//0 otherwise
int validateuser(MYSQL *conn, char msg[50])
{
	//Retrive username
	char user[20], pass[20];
	int i=0;
	while(msg[i] != ' ' &&  i<20)
	{
		user[i]=msg[i];
		i++;
	}
	user[i]='\0';
	if(i>20)
	{
		printf("Illegal user name");
		return -1;
	}
	i+=5;	//length of ' and ' = 5
	int j=0;
	while(msg[i])
	{
		pass[j]=msg[i];
		j++;
		i++;
	}
	pass[j]='\0';
	printf("Username : %s",user);
	printf("Password : %s",pass);

	char query[100]="SELECT * FROM USER WHERE id = '";
	strcat(query,user);
	puts(query);
	strcat(query,"' AND password = '");
	puts(query);
	strcat(query,pass);
	strcat(query,"'");
	puts(query);
	if (mysql_query(conn, query)) 
	{
        fprintf(stderr, "%s\n", mysql_error(conn));
 	    return -1;
	}

	MYSQL_RES *result = mysql_store_result(conn);

	if (result == NULL) 
	{		
		printf("\n Error occurred");
	  exit(1);
	}

	MYSQL_ROW row;
  
    if ((row = mysql_fetch_row(result))) 
    { 
		mysql_free_result(result);
		return 1;
    }
	return 0;
}

int registeration(MYSQL *conn, char msg[50])
{
	//Retrive username
	char user[20], pass[20];
	int i=0;
	while(msg[i] != ' ' &&  i<20)
	{
		user[i]=msg[i];
		i++;
	}
	user[i]='\0';
	if(i>20)
	{
		printf("Illegal user name");
		return -1;
	}
	i+=5;	//length of ' and ' = 5
	int j=0;
	while(msg[i])
	{
		pass[j]=msg[i];
		j++;
		i++;
	}
	pass[j]='\0';
	printf("Username : %s",user);
	printf("Password : %s",pass);

	char query[100]="INSERT INTO USER VALUES ('";
	strcat(query,user);
	puts(query);
	strcat(query,"', '");
	puts(query);
	strcat(query,pass);
	strcat(query,"')");
	puts(query);
	if (mysql_query(conn, query)) 
	{
        fprintf(stderr, "%s\n", mysql_error(conn));
 	    return -1;
	}
	return 1;
}

void *client_service(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char client_message[2000];
     
    //Database connection
    MYSQL *conn;
   MYSQL_RES *res;
   MYSQL_ROW row;
   char *server = "localhost";
   char *user = "root";
   char *password = ""; /* set me first */
   char *database = "UNP";
   conn = mysql_init(NULL);
   /* Connect to database */
   if (!mysql_real_connect(conn, server,
         user, password, database, 0, NULL, 0)) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      exit(1);
   }

   bzero(client_message, 2000);
   strcpy(client_message, "Do you want to Register or Log in ");
   write(sock , client_message , strlen(client_message));
   bzero(client_message, 2000);
   recv(sock , client_message , 2000 , 0);

   if(strcmp(client_message,"Register") == 0 || strcmp(client_message,"register") == 0)
   {
   		bzero(client_message, 2000);
   		strcpy(client_message, "Enter your username and password ");
   		write(sock , client_message , strlen(client_message));
   		bzero(client_message, 2000);
   		recv(sock , client_message , 2000 , 0);
   		if(registeration(conn, client_message) != -1)
   		{
	   		bzero(client_message, 2000);
	   		strcpy(client_message, "Registered successfully \nEnter your username and password to log in");
	   		write(sock , client_message , strlen(client_message));
   		}
   		else
   		{
   			bzero(client_message, 2000);
	   		strcpy(client_message, "Error occurred. Please try again");
	   		write(sock , client_message , strlen(client_message));
	   		exit(1);
   		}
   }
   else
   {
   		bzero(client_message, 2000);
    	strcpy(client_message, "Enter username and passowrd to login ");
    	write(sock , client_message , strlen(client_message));
    }
    //Login user
    int userstatus = 0;		//User not valid
    while(userstatus != 1)
    {
    	char username[20], pass[20];
    	bzero(client_message, 2000);
    	recv(sock , client_message , 2000 , 0);
    	strcpy(username, client_message);
     
    	userstatus = validateuser(conn, client_message);
    	if(userstatus == 0)
    	{
    		bzero(client_message, 2000);
    		strcpy(client_message, "Invalid Username or Password ");
    		write(sock , client_message , strlen(client_message));
    	}
    }

    bzero(client_message, 2000);
	strcpy(client_message, "Welcome!!!");
	write(sock , client_message , strlen(client_message));
	bzero(client_message, 2000);



    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //end of string marker
		//client_message[read_size] = '\0';
		puts("Sent 1: ");

		puts(client_message);
	

		//Send the message back to client
        write(sock , client_message , strlen(client_message));
		
		//clear the message buffer
		bzero(client_message, 2000);

		puts("Sent 2: ");

		puts(client_message);
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    return 0;
} 