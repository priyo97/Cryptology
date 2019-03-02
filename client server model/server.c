#include <stdio.h>		// for printf() fgets() functions
#include <sys/types.h>	// datatypes for using functions declared in sys/socket.h
#include <sys/socket.h> // for socket() function and sockaddr struct 
#include <netinet/in.h> // for sockaddr_in struct 
#include <stdlib.h>		// for atoi() function 
#include <string.h>		// for bzero() and bcopy() functions
#include <unistd.h>		// for read() and write() function

void error(char *msg)
{
	printf("%s",msg);
	exit(1);
}


int main(int argc, char **argv)
{
	int sockfd, newsockfd, portno, clilen, n;

	char buffer[256];

	struct sockaddr_in serv_addr, cli_addr;
	
	if( argc < 2 )
		
		error("Usage: ./server [PORT NUMBER]");

	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if( sockfd < 0 )
		
		error("ERROR: socket not created");

	
		
	bzero((char*)&serv_addr,sizeof(serv_addr));

	portno = atoi(argv[1]);

	


	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	
	n = bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));


	if(n < 0)

		error("Error: Binding Unsuccessful");


	listen(sockfd,5);

	
	clilen = sizeof(cli_addr);

	printf("Server Started\n");


	newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr,&clilen);


	if(newsockfd < 0)
		
		error("ERROR: on accept");



	while(1)
	{
		bzero(buffer,256);
	
		n = read(newsockfd,buffer,255);

		if(n < 1)
			
			error("Error: reading from socket");


		printf("From client: %s\n",buffer);


	
		bzero(buffer,256);

		printf("Enter Message: ");
		
		fgets(buffer,255,stdin);
		
		n = write(newsockfd,buffer,strlen(buffer));

		if (n < 1)
		
			error("Error: writing to socket");

	}


	close(newsockfd);
	close(sockfd);
}
