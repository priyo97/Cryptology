#include <stdio.h>		// for printf() fgets() functions
#include <sys/types.h>	// datatypes for using functions declared in sys/socket.h
#include <sys/socket.h> // for socket() function and sockaddr struct 
#include <netinet/in.h> // for sockaddr_in struct 
#include <stdlib.h>		// for atoi() function 
#include <string.h>		// for bzero() and bcopy() functions
#include <unistd.h>		// for read() and write() function
#include <netdb.h>		// for using the function gethostbyname() and hostent struct

void error(char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char **argv)
{
	int sockfd, portno, n;

	struct sockaddr_in serv_addr;

	struct hostent *server;

	char buffer[256];



	if(argc < 3)
	
		error("Usage: ./client [SERVER ADDRESS] [SERVER PORT] | Usage: ./client [SERVER DOMAIN NAME] [SERVER PORT]");

	

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	

	if(sockfd < 0)
		
		error("Error: Socket not created");


	



	server = gethostbyname(argv[1]);

	
	if(server == NULL)
		
		error("Error: No such host");

	
	portno = atoi(argv[2]);
	

	bzero((char*)&serv_addr,sizeof(serv_addr));

	
	serv_addr.sin_family = AF_INET;
	
	bcopy((char*)server -> h_addr, (char*)&serv_addr.sin_addr.s_addr,server-> h_length);

	serv_addr.sin_port = htons(portno);





	n = connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));

	if(n < 0)
		
		error("Error: Could not connect");
	
	else
		
		printf("Connected to server...\n");


	while(1)
	{
		bzero(buffer,256);

		printf("Enter message: ");
		
		fgets(buffer,255,stdin);
	
		

		n = write(sockfd,buffer,strlen(buffer));
	
		if(n < 1)
		
			error("Error: writing to socket");


		bzero(buffer,256);

		n = read(sockfd,buffer,255);

		if(n < 1)

			error("Error: reading from socket");

		printf("From server: %s\n",buffer);

	}


	close(sockfd);
			
}
