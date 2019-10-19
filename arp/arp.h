#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_arp.h>

typedef struct arph
{
    uint16_t hdw_type;
    uint16_t proto_type;
    uint8_t  hdw_addr_len;
    uint8_t  proto_addr_len;
    uint16_t opcode;
    uint8_t sender_hdw_addr[6];
    uint8_t sender_proto_addr[4];
    uint8_t target_hdw_addr[6];
    uint8_t target_proto_addr[4];
}__attribute__((packed))arph;

typedef struct arp_packet
{
    struct ethhdr l2;
    arph          l3;
}__attribute__((packed))arp_packet;


#define ETHERNET_HEADER_LEN     sizeof(struct ethhdr)
#define ARP_PROTO_HEADER_LEN    sizeof(arph)
#define ARP_PACKET_LEN          sizeof(arp_packet)


int create_socket(const char *interface, unsigned int protocol)
{
    int sock = socket(AF_PACKET, SOCK_RAW, htons(protocol));

    if(sock == -1)
    {  
        printf("Error: %s", strerror(errno));
        exit(1);
    }

    struct sockaddr_ll sock_addr;
    memset(&sock_addr, 0, sizeof(struct sockaddr_ll));

    sock_addr.sll_family   = AF_PACKET;
    sock_addr.sll_protocol = htons(protocol);
    sock_addr.sll_ifindex  = if_nametoindex(interface);
    sock_addr.sll_pkttype  = PACKET_HOST;


    if(bind(sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) == -1)
    { 
        printf("Error: %s", strerror(errno));
        exit(1);
    }

    return sock; 
}

void* create_arp_packet(struct ethhdr l2, arph l3)
{
    void *packet = malloc(ARP_PACKET_LEN);
    memcpy(packet, &l2, ETHERNET_HEADER_LEN);
    memcpy(packet + ETHERNET_HEADER_LEN, &l3, ARP_PROTO_HEADER_LEN);

    return packet;
}

/*
struct ethhdr {
    unsigned char   h_dest[ETH_ALEN];  
    unsigned char   h_source[ETH_ALEN]; 
    __be16      h_proto; 
} __attribute__((packed));

*/

struct ethhdr generate_l2_header(int sock, const char *intf_name)
{
    int i;

    struct ethhdr l2;

    memset(&l2, 0, sizeof(struct ethhdr));

    struct ifreq intf_details;

    memset(&intf_details, 0, sizeof(struct ifreq));

    strncpy(intf_details.ifr_name, intf_name, IFNAMSIZ);

    ioctl(sock, SIOCGIFHWADDR, &intf_details);


    for(i=0;i<6;i++)
    {
        l2.h_source[i] = (unsigned char) intf_details.ifr_hwaddr.sa_data[i];
    }

    for(i=0;i<6;i++)
    {
        l2.h_dest[i] = 255;
    }

    l2.h_proto = htons(ETH_P_ARP);

    return l2;
}

/*

typedef struct arph
{
    uint16_t hdw_type;
    uint16_t proto_type;
    uint8_t  hdw_addr_len;
    uint8_t  proto_addr_len;
    uint16_t opcode;
    uint8_t sender_hdw_addr[6];
    uint8_t sender_proto_addr[4];
    uint8_t target_hdw_addr[6];
    uint8_t target_proto_addr[4];
}__attribute__((packed))arph;
*/

arph generate_l3_header(struct ethhdr l2, unsigned char *src_ip, unsigned char *dst_ip)
{
    arph l3;

    l3.hdw_type       = htons(ARPHRD_ETHER);
    l3.proto_type     = htons(ETH_P_IP);
    l3.hdw_addr_len   = 6;
    l3.proto_addr_len = 4;

    l3.opcode = htons(ARPOP_REQUEST);
    
    memcpy(&l3.sender_hdw_addr, l2.h_source, 6);
    memcpy(&l3.sender_proto_addr, src_ip, 4);


    memset(&l3.target_hdw_addr, 0, 6);
    memcpy(&l3.target_proto_addr, dst_ip, 4);

    return l3;
}

void print_l3_header(arph l3)
{
    printf("Hardware type : %d\n", ntohs(l3.hdw_type));
    printf("Protocol type : %d\n", ntohs(l3.proto_type));
    printf("Hw addr len   : %d\n", l3.hdw_addr_len);
    printf("Proto addr len: %d\n", l3.proto_addr_len);
    printf("Opcode        : %d\n", ntohs(l3.opcode));
    printf("src MAC addr  : %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", l3.sender_hdw_addr[0], l3.sender_hdw_addr[1],
                                                                l3.sender_hdw_addr[2], l3.sender_hdw_addr[3],
                                                                l3.sender_hdw_addr[4], l3.sender_hdw_addr[5]);


    printf("src IP addr   : %d.%d.%d.%d\n", l3.sender_proto_addr[0], l3.sender_proto_addr[1],
                                            l3.sender_proto_addr[2], l3.sender_proto_addr[3]);

    printf("dst MAC addr  : %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", l3.target_hdw_addr[0], l3.target_hdw_addr[1],
                                                                l3.target_hdw_addr[2], l3.target_hdw_addr[3],
                                                                l3.target_hdw_addr[4], l3.target_hdw_addr[5]);

    printf("dst IP addr   : %d.%d.%d.%d\n", l3.target_proto_addr[0], l3.target_proto_addr[1],
                                            l3.target_proto_addr[2], l3.target_proto_addr[3]);
}

void print_l2_header(struct ethhdr l2)
{

    printf("source mac addr: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", l2.h_source[0], l2.h_source[1], l2.h_source[2],
                                                                l2.h_source[3], l2.h_source[4], l2.h_source[5]);

    printf("Dest mac addr  : %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", l2.h_dest[0], l2.h_dest[1], l2.h_dest[2],
                                                                l2.h_dest[3], l2.h_dest[4], l2.h_dest[5]);

    printf("L3 proto       : %.2x%.2x\n", l2.h_proto & 0xFF, (l2.h_proto & 0xFF00) >> 8);
}

int print_arp_reply_packet(void *packet)
{
    struct ethhdr *l2 = (struct ethhdr*) packet;
    arph          *l3 = (arph*)(packet + ETHERNET_HEADER_LEN);

    if( l3 -> opcode == htons(ARPOP_REPLY) )
    {
        print_l2_header(*l2);
        print_l3_header(*l3);
        return 1;
    }

    return 0;
}
