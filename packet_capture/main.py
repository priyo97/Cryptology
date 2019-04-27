'''
Objective:  This application captures packets, 
            converts each packet into a bytearray, 
            extracts the header details at each layer 2, layer 3, layer 4,
            checks whether the packet checksum is right or wrong by recomputing the checksum,
            stores the data into a dictionary: pkt_details (can be exported as json if needed),
            prints the details of each packet


Usage: ./main.py [interface_name] [n_pkts]

interface_name: Name of the interface to capture packets from
n_pkts        : Number of packets to capture

e.g. - ./main.py eth1 2

N.B. -  Scapy library is used to capture packets. 
        But normal raw sockets can also be used to do the same. 
        Just need to change the capture_packet function to implemenet a custom packet capture function.


Current State:

1. Layer 2 : Can parse Ethernet header
2. Layer 3 : Can parse IPV4 header
3. layer 4 : Can parse TCP, UDP, ICMP header     

'''


import sys
from scapy.all import *
from extractor import *
from print_details import *

def capture_packet(iface, n_pkts):
    
    pkt_list = sniff(iface=iface, count=n_pkts)

    raw_pkt_list = (raw(pkt) for pkt in pkt_list)

    return raw_pkt_list

def main(argv):

    ETH_HEADER_LEN = 14
    ETH_P_IP = "0x0800"

    raw_pkt_list = capture_packet(iface=argv[0], n_pkts=int(argv[1]))
    

    for raw_pkt in raw_pkt_list:
        
        buf = bytearray(raw_pkt)

        pkt_details = {}

        # OSI Model Layer 2
        get_l2_headers(buf, pkt_details)
        print_l2_headers(pkt_details)

        if pkt_details["l2"]["h_proto"] == ETH_P_IP:

            # OSI Model Layer 3
            get_l3_headers(buf[ETH_HEADER_LEN:], pkt_details)
        
            if pkt_details["l3"]:
                print_l3_headers(pkt_details)

            if  pkt_details["l3"]:
                calculate_l3_checksum(buf[ETH_HEADER_LEN:], pkt_details["l3"]["total_ip_header_len"])

            # OSI Model Layer 4
            if pkt_details["l3"]["l4_protocol"] in {1, 6, 17}:
        
                get_l4_headers( buf[ETH_HEADER_LEN + pkt_details["l3"]["total_ip_header_len"]:], 
                                pkt_details, 
                                pkt_details["l3"]["l4_protocol"])

                print_l4_headers(pkt_details, pkt_details["l3"]["l4_protocol"])

                pseudo_header = create_pseudo_header(buf[ETH_HEADER_LEN:], pkt_details["l3"]["transport_seg_len"])
        
                calculate_l4_checksum(  buf[ETH_HEADER_LEN + pkt_details["l3"]["total_ip_header_len"]:], 
                                        pseudo_header, 
                                        pkt_details["l3"]["l4_protocol"])


if __name__ == "__main__":

    if len(sys.argv) < 3:
        usage(sys.argv[0])

    main(sys.argv[1:])

