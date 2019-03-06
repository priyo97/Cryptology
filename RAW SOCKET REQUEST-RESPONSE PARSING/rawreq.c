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

#define MODE "w"
#define IP "192.168.4.159"

int main() 
{    
    int sock = create_raw_socket("eth1",ETH_P_ALL);
	

    unsigned char buffer[65536];

	ssize_t recv_size = -1;




    int data_start = 0;

    int header_len, body_len, i, header_found = 0, body_found = 0;

    unsigned char *tmp, *header_portion, *body_portion;

    


    header_portion = (unsigned char*) malloc(65536 * sizeof(unsigned char));

    body_portion = (unsigned char*) malloc(65536 * sizeof(unsigned char));

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


        struct tcphdr *tcp = (struct tcphdr*)( buffer + sizeof(*ip) + sizeof(*eth));


	    if( strcmp(inet_ntoa(source.sin_addr),IP) == 0 && tcp -> fin )
        {
            break;
        }


	    if( strcmp(inet_ntoa(dest.sin_addr),IP) == 0 )
	    {


		    unsigned char *data = (unsigned char*)(buffer + sizeof(*tcp) + sizeof(*ip) + sizeof(*eth));
		
            int len = recv_size - sizeof(*tcp) - sizeof(*ip) - sizeof(*eth);


            if(!data_start && data[0] == 'P')
                
                data_start = 1;


            if(data_start)
            {

                if(!header_found)
                {

                    tmp = strstr(data,"\r\n\r\n");


                    header_portion = (unsigned char*) realloc(header_portion, header_len + (tmp + 3 - data) );
                    
                    if(tmp != NULL)
                    {    
                        tmp[2] = '\0';

                        tmp = tmp + 4;

        

                        strcpy(header_portion + header_len, data);

                        header_len += strlen(data);

                        header_found = 1;





                        len = len - strlen(data) - 2;

                        body_portion = (unsigned char*) realloc(body_portion, body_len + len );
                        
                        for( i = 0 ; i < len ; i++)
                        {
                            body_portion[body_len++] = tmp[i];
                        }
                        

                        body_found = 1;
        

                    }
                    else
                    {
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


                    body_portion = (unsigned char *) realloc(body_portion, body_len + len );

                    for( i = 0 ; i < len ; i++)
                    {
                        body_portion[body_len++] = data[i];
                    }



                }

            }
		
    	}

    }


    //printf("Head-portion:\n\n%s\n", header_portion);
    
    //printf("Body-portion:\n");

    struct form_data data;

    extract_form_fields(header_portion, header_len, body_portion, body_len, &data);



    printf("%d\n",data.num_form_fields);

    for(i = 0 ; i < data.num_form_fields ; i++)
    {
        
        if(data.f[i].is_file)
        {
            FILE *fptr = fopen(data.f[i].filename,MODE);

            if(fptr == NULL)
                
                error("Error with file opening");
                
            fwrite(data.f[i].value, sizeof(unsigned char), data.f[i].length, fptr);

            fflush(fptr);
            fclose(fptr);
        }
    }



    free(data.f);    
    
    free(header_portion);
    free(body_portion);


    close(sock);
}
