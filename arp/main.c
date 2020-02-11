#include <stdio.h>
#include <stdlib.h>
#include "arp.h"

#define INTERFACE_NAME "wlp1s0"

int main()
{
    printf("Ethernet header len : %ld\n", ETHERNET_HEADER_LEN);
    printf("ARP proto header len: %ld\n", ARP_PROTO_HEADER_LEN);
    printf("ARP proto packet len: %ld\n", ARP_PACKET_LEN);

    int sock = create_socket(INTERFACE_NAME, ETH_P_ARP);
    
    struct ethhdr l2;

    l2 = generate_l2_header(sock, INTERFACE_NAME);
    
    arph l3;

    unsigned char src_ip[] = {172, 11, 13, 66};

    unsigned char dst_ip[] = {172, 11, 12, 184};

    l3 = generate_l3_header(l2, src_ip, dst_ip);

   
    /* Send ARP request packet */ 
    void *packet = create_arp_packet(l2, l3);
    
    print_arp_reply_packet(packet);

    long sent_size = send(sock, packet, ARP_PACKET_LEN, 0);

    free(packet);

    printf("------------------------------------------\n");

    /* Recv ARP reply packet */
    packet = malloc(100);

    while(1)
    {
        memset(packet, 0, 100);
    
        long recv_size = recv(sock, packet, 100, 0);

        if(print_arp_reply_packet(packet) == 1)
        {
            break;
        }
    }

    free(packet);
    close(sock);
}
