#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<net/if.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<linux/if_ether.h>
#include<linux/if_packet.h>
#include<linux/ip.h>
#include<linux/tcp.h>
#include"helper.h"



#define FILENAME "a.png"
#define MODE "w"

int main() 
{    


    int sock = create_raw_socket("eth0",ETH_P_ALL);
	

    unsigned char buffer[65536];

	ssize_t recv_size = -1;




    int data_start = 0;

    int header_len, body_len, i, header_found = 0, body_found = 0;

    unsigned char *tmp, *header_portion, *body_portion;

    
    header_len = body_len = 0;
	

    while(1)
	{
		bzero(&buffer,sizeof(buffer));
        	
		recv_size = recv(sock, &buffer,65536, 0);
        	
		if (recv_size < 1)
        {
            break;
		}


		struct ethhdr *eth = (struct ethhdr *)(buffer);

		struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));



		struct sockaddr_in source, dest;

		
	    bzero(&source, sizeof(source));
		bzero(&dest, sizeof(dest));


		source.sin_addr.s_addr = ip -> saddr;
		dest.sin_addr.s_addr = ip -> daddr; 




	    if( strcmp(inet_ntoa(source.sin_addr),"54.192.216.30") == 0 || 
            strcmp(inet_ntoa(source.sin_addr),"54.192.216.20") == 0 || 
            strcmp(inet_ntoa(source.sin_addr),"54.192.216.56") == 0 ||
            strcmp(inet_ntoa(source.sin_addr),"54.192.216.215") == 0 )
	    {
		    
            struct tcphdr *tcp = (struct tcphdr*)( buffer + sizeof(*ip) + sizeof(*eth));

		    unsigned char *data = (unsigned char*)(buffer + sizeof(*tcp) + sizeof(*ip) + sizeof(*eth));
		
	    	int len = recv_size - sizeof(*tcp) - sizeof(*ip) - sizeof(*eth);



            if(!data_start && data[0] == 'H')
                
                data_start = 1;


            if(data_start)
            {

                if(!header_found)
                {
                    tmp = strstr(data,"\r\n\r\n");

                    if(tmp != NULL)
                    {
                        
                        header_portion = (unsigned char*) realloc(header_portion, header_len + (tmp + 3 - data) );
                        
                        tmp[2] = '\0';

                        tmp = tmp + 4;

    

                        strcpy(header_portion + header_len, data);

                        header_len += strlen(data);

                        header_found = 1;





                        len = len - strlen(data) - 2;

                        body_portion = (unsigned char*) realloc( body_portion, body_len + len );
                        
                        for( i = 0 ; i < len ; i++)
                        {
                            body_portion[body_len++] = tmp[i];
                        }
                        

                        body_found = 1;
        

                    }
                    else
                    {

                        header_portion = (unsigned char*) realloc(header_portion, header_len + (tmp - data + 3) );
                        strcpy(header_portion + header_len, data);
                        header_len += strlen(data);
                    }            



                }
                else if(body_found)
                {
                   

                    if( strcmp(data,"") == 0 || tcp -> fin )
                    {
                        break;
                    }


                    body_portion = (unsigned char *) realloc( body_portion, body_len + len );
        

                    for( i = 0 ; i < len ; i++)
                    {
                        body_portion[body_len++] = data[i];
                    }



                }

            }
		
    	}

    }


    printf("\nHeader-length: %d bytes\n",header_len);


    struct header_fields http_hdr;


    extract_header_fields(header_portion, header_len, &http_hdr);


    tmp = get_header_value("Content-Type", &http_hdr);

 

    printf("Content-type --->%s\n", tmp);

    printf("\nBody-length: %d bytes\n",body_len);

//  printf("Body-portion: \n%s\n",body_portion);


   



    FILE *fptr = fopen(FILENAME, MODE);

    if(fptr == NULL)

        error("Error opening file");

    
    fwrite(body_portion,sizeof(body_portion[0]),body_len,fptr);

    
    fflush(fptr);

    fclose(fptr);






    free(header_portion);
    free(body_portion);


    free(http_hdr.keys);
    free(http_hdr.values);

    close(sock);
}
