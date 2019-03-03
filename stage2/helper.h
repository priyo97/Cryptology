// contains helper functions related to raw sockets
// functions used from libraries
/*

    stdio.h
    string.h
    sys/socket.h
    sys/types.h
    net/if.h
    linux/if_packet.h

*/


struct header_fields{

    unsigned char **keys;
    unsigned char **values;
    int num_headers;
};

struct form_field{

    char *name;
    short is_file;
    char *filename;
    char *value;
};

struct form_data{

    struct form_field *f;
    int num_form_fields;

};

void error(char *msg)
{
    perror(msg);
    exit(1);
}


int create_raw_socket(char *interface, unsigned char protocol)
{
    int sock = socket(AF_PACKET, SOCK_RAW, htons(protocol));

    if(sock == -1)
    {
        error("Failed to create socket");    
    }
    else
    {
        printf("Socket successfully created\n");
    }
        
    
    struct sockaddr_ll socket_address;

    bzero((char*)&socket_address, sizeof(socket_address));




    socket_address.sll_family = AF_PACKET;

    socket_address.sll_ifindex = if_nametoindex(interface);

    socket_address.sll_protocol = htons(protocol);



    int check = bind(sock, (struct sockaddr*) &socket_address, sizeof(socket_address));


    if(check < 0)

        error("Binding Error");


    return sock;

}


char* get_header_value(char *searchkey, struct header_fields *http_hdr)
{
    
    register int i;

    for(i = 0 ; i < http_hdr -> num_headers ; i++)
    {
        if( strcmp( searchkey, http_hdr -> keys[i] ) == 0 )
        {
            return http_hdr -> values[i];
        }
        
    }
    
    return NULL;
}




void extract_header_fields(char *header_portion, int header_len, struct header_fields *http_hdr)
{
    unsigned char **headers = malloc(100 * sizeof(unsigned char*));


    // num_headers to keep track of number of headers
    // j to keep track of the beginning of the header
    register int i, j, num_headers;

    
    num_headers = j = 0;


    for(i = 0 ; i < header_len ; i++ )
    {
        if(header_portion[i] == '\r')
        {
            
            header_portion[i] = '\0';

            headers[num_headers] = header_portion + j;

            j = i + 2;

            num_headers++ ;

            i++;
        }
    }

    unsigned char **keys   = malloc( (num_headers - 1) * sizeof(unsigned char *));
    unsigned char **values = malloc( (num_headers - 1) * sizeof(unsigned char *));

    unsigned char *tmp;

    for(i = 1 ; i < num_headers ; i++)
    {
        tmp = headers[i];
    
        j = 0;

        while(tmp[j] != ':')
        {
            j++;
        }

        tmp[j] = '\0';
    
        // i - 1 cause i is starting from 1 to avoid the HTTP response code stored at index 0 of headers and I want to insert values from index 0

        keys[i-1] = tmp;
        values[i-1] = (tmp + j + 2);
    }

    
    free(headers);

    

    http_hdr -> keys = keys;
    http_hdr -> values = values;
    http_hdr -> num_headers = num_headers - 1;

}



void extract_form_fields(char* header_portion,int header_len,char *body_portion, struct form_data *f)
{
    unsigned char *tmp;

    struct header_fields http_hdr;

    extract_header_fields(header_portion, header_len, &http_hdr);

    tmp = get_header_value("Content-Type", &http_hdr);

    tmp = strstr(tmp,"=") + 1;


    unsigned char delimiter[100] = "--";

    strcat( (char*) delimiter, (const char*) tmp );


    tmp = body_portion;

    unsigned char *token, *t, *s, *r, *q;

    int i = 0;

    struct form_field *form_data = malloc(20 * sizeof(struct form_field));

    while(1)
    {
        token = strstr(tmp, (const char*) delimiter);

        if(token == NULL)
        {
            break;
        }


        token[0] = '\0';

        if( strcmp(tmp,"") != 0 )
        {
            t = strstr(tmp,"\r\n\r\n");

            t[0] = '\0';

            t = t + 4;

            s = strstr(tmp,"filename=\"");

            if(s != NULL)
            {

                r = strstr(tmp,"name=\"");

                q = strstr(r + 6,"\"");

                q[0] = '\0';
        
                q = strstr(s + 10,"\"");

                q[0] = '\0';

                
                form_data[i].name = (r + 6);
                form_data[i].is_file = 1;
                form_data[i].filename = (s + 10);
                form_data[i].value = t;   
            }
            else
            {
                r = strstr(tmp,"name=\"");

                q = strstr(r + 6,"\"");

                q[0] = '\0';

                form_data[i].name = (r + 6);
                form_data[i].is_file = 0;
                form_data[i].filename = NULL;
                form_data[i].value = t;

            }

            i++;
        }

        tmp = token + strlen(delimiter) + strlen("\r\n");
    }


    free(http_hdr.keys);
    free(http_hdr.values);



  //  printf("\n##############################################\n");
  //  printf("Name    : %s\n",form_data[2].name);
  //  printf("Filename: %s\n",form_data[2].filename);
  //  printf("%s\n",form_data[2].value);

    f -> f = form_data;
    f -> num_form_fields = i;
}
