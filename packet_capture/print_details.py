def usage(program_name):
    print("Usage: ./%s [interface_name] [n_pkts]" %(program_name))
    exit(1)

def print_l2_headers(pkt_details):
    
    print("Source MAC address: %s" % pkt_details["l2"]["h_source"])
    print("Dest   MAC address: %s" % pkt_details["l2"]["h_dest"])
    print("Ethernet Protocol : %s" % pkt_details["l2"]["h_proto"])
    print("------------------------------------------------------")


def print_l3_headers(pkt_details):

    IPV4_PROTO = 4

    if pkt_details["l3"]["version"] == IPV4_PROTO:
   
        print("IP version        : %d" % (pkt_details["l3"]["version"]))
        print("IP Header DWORDS  : %d" % (pkt_details["l3"]["ihl"]))
        print("IP Header length  : %d bytes" % (pkt_details["l3"]["total_ip_header_len"]))
        print("IP tos            : %d" % (pkt_details["l3"]["tos"]))
        print("IP total len      : %d" % (pkt_details["l3"]["total_len"]))
        print("IP ttl            : %d" % (pkt_details["l3"]["ttl"]))
        print("IP protocol       : %d" % (pkt_details["l3"]["l4_protocol"]))
        print("Source IP         : %s" % (pkt_details["l3"]["source_ip"]))
        print("Dest IP           : %s" % (pkt_details["l3"]["dest_ip"]))
        print("IP Checksum       : %d" % (pkt_details["l3"]["checksum"]))
        print("IP OPTIONS PRESENT: %s" % (pkt_details["l3"]["options_present"]))
        print("------------------------------------------------------")

        return pkt_details["l3"]["total_ip_header_len"], pkt_details["l3"]["l4_protocol"]

    return None, None


def print_l4_headers(pkt_details, l4_proto):

    TCP_PROTO  = 6
    UDP_PROTO  = 17
    ICMP_PROTO = 1

    if l4_proto == TCP_PROTO:

        print("Source Port    : %d" %(pkt_details["l4"]["source_port"]))
        print("Dest   Port    : %d" %(pkt_details["l4"]["dest_port"]))
        print("Seq Number     : %d" %(pkt_details["l4"]["seq_num"]))
        print("Ack number     : %d" %(pkt_details["l4"]["ack_num"]))
        print("HLEN           : %d" %(pkt_details["l4"]["hlen"]))
        print("Total Hlen     : %d bytes" %(pkt_details["l4"]["tot_header_len"]))
        print("----------------Flags----------------")
        print("| fin | syn | rst | psh | ack | urg |")
        print("-------------------------------------")
        print("|  %d  |  %d  |  %d  |  %d  |  %d  |  %d  |" %(  pkt_details["l4"]["flags"]["fin"], 
                                                                pkt_details["l4"]["flags"]["syn"], 
                                                                pkt_details["l4"]["flags"]["rst"], 
                                                                pkt_details["l4"]["flags"]["psh"], 
                                                                pkt_details["l4"]["flags"]["ack"], 
                                                                pkt_details["l4"]["flags"]["urg"] ))
        print("-------------------------------------")
        print("Window Size    : %d" %(pkt_details["l4"]["window_size"]))
        print("Checksum       : %d" %(pkt_details["l4"]["checksum"]))
        print("Options present: %s" %(pkt_details["l4"]["options_present"]))

    elif l4_proto == UDP_PROTO:
    
        print("Source Port             : %d" %(pkt_details["l4"]["source_port"]))
        print("Dest   Port             : %d" %(pkt_details["l4"]["dest_port"]))
        print("Tot len (header + data) : %d" %(pkt_details["l4"]["tot_seg_len"]))
        print("Checksum                : %d" %(pkt_details["l4"]["checksum"]))

    elif l4_proto == ICMP_PROTO:
        
        print("ICMP Type : %d" %(pkt_details["l4"]["icmp_type"]))
        print("ICMP Code : %d" %(pkt_details["l4"]["icmp_code"]))
        print("Checksum  : %d" %(pkt_details["l4"]["checksum"]))

    else:
        print("Unknown L4 PROTOCOL")

    print("-------------------------END--------------------------")
