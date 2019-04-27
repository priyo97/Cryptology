def get_l2_headers(buf, pkt_details):
    
    pkt_details["l2"] = {}
    pkt_details["l2"]["h_source"] = "%.2x.%.2x.%.2x.%.2x.%.2x.%.2x" % (buf[0], buf[1], buf[2], buf[3], buf[4] , buf[5])
    pkt_details["l2"]["h_dest"]   = "%.2x.%.2x.%.2x.%.2x.%.2x.%.2x" % (buf[6], buf[7], buf[8], buf[9], buf[10], buf[11])
    pkt_details["l2"]["h_proto"]  = "0x%.2x%.2x" % (buf[12],buf[13])
    pkt_details["l2"]["tot_len"]  = len(buf)

def get_l3_headers(buf, pkt_details):

    MIN_IPV4_HEADER_LEN = 20
    IPV4_PROTO = 4
    
    pkt_details["l3"] = {}

    ver = (buf[0] & 0xF0) >> 4

    pkt_details["l3"]["version"] = ver

    if ver == IPV4_PROTO:
        
        pkt_details["l3"]["ihl"]                 = (buf[0] & 0x0F)
        pkt_details["l3"]["total_ip_header_len"] = pkt_details["l3"]["ihl"] * 4
        pkt_details["l3"]["tos"]                 = buf[1]
        pkt_details["l3"]["total_len"]           = (buf[2] << 8) + buf[3]
        pkt_details["l3"]["ttl"]                 = buf[8]
        pkt_details["l3"]["l4_protocol"]         = buf[9]
        pkt_details["l3"]["checksum"]            = (buf[10] << 8) + buf[11]
        pkt_details["l3"]["source_ip"]           = "%d.%d.%d.%d" % (buf[12], buf[13], buf[14], buf[15])
        pkt_details["l3"]["dest_ip"]             = "%d.%d.%d.%d" % (buf[16], buf[17], buf[18], buf[19])
        pkt_details["l3"]["options_present"]     = pkt_details["l3"]["total_ip_header_len"] > MIN_IPV4_HEADER_LEN
        pkt_details["l3"]["transport_seg_len"]   = pkt_details["l3"]["total_len"] - pkt_details["l3"]["total_ip_header_len"]

    else:
        
        pkt_details["l3"] = False



def create_pseudo_header(buf, transport_segment_len):

    pseudo_header = bytearray()
    
    pseudo_header.extend(buf[12:20])

    pseudo_header.extend(bytes( [0] ))
    pseudo_header.extend(bytes( [buf[9]] ))

    msb = (transport_segment_len & 0xFF00) >> 8
    lsb = (transport_segment_len & 0x00FF)

    pseudo_header.extend(bytes([msb, lsb]))

    return pseudo_header
    

def get_l4_headers(buf, pkt_details, l4_proto):

    TCP_PROTO  = 6
    UDP_PROTO  = 17
    ICMP_PROTO = 1

    pkt_details["l4"] = {}

    if l4_proto == TCP_PROTO:

        MIN_TCP_HEADER_LEN = 20
        
        pkt_details["l4"]["source_port"]    = (buf[0] << 8) + buf[1]
        pkt_details["l4"]["dest_port"]      = (buf[2] << 8) + buf[3]
        pkt_details["l4"]["seq_num"]        = (buf[4] << 24) + (buf[5] << 16) + (buf[6]  << 8) + buf[7]
        pkt_details["l4"]["ack_num"]        = (buf[8] << 24) + (buf[9] << 16) + (buf[10] << 8) + buf[11]
        pkt_details["l4"]["hlen"]           = (buf[12] & 0xF0) >> 4
        pkt_details["l4"]["tot_header_len"] = pkt_details["l4"]["hlen"] * 4
        pkt_details["l4"]["flags"] = {

            "fin": (buf[13] & 0x3F) & (1 << 0) != 0,
            "syn": (buf[13] & 0x3F) & (1 << 1) != 0,
            "rst": (buf[13] & 0x3F) & (1 << 2) != 0,
            "psh": (buf[13] & 0x3F) & (1 << 3) != 0,
            "ack": (buf[13] & 0x3F) & (1 << 4) != 0,
            "urg": (buf[13] & 0x3F) & (1 << 5) != 0
        }

        pkt_details["l4"]["window_size"]     = (buf[14] << 8) + buf[15]
        pkt_details["l4"]["checksum"]        = (buf[16] << 8) + buf[17]
        pkt_details["l4"]["options_present"] = pkt_details["l4"]["tot_header_len"] > MIN_TCP_HEADER_LEN 

    elif l4_proto == UDP_PROTO:
        
        pkt_details["l4"]["source_port"] = (buf[0] << 8) + buf[1]
        pkt_details["l4"]["dest_port"]   = (buf[2] << 8) + buf[3]
        pkt_details["l4"]["tot_seg_len"] = (buf[4] << 8) + buf[5]
        pkt_details["l4"]["checksum"]    = (buf[6] << 8) + buf[7]

    elif l4_proto == ICMP_PROTO:

        pkt_details["l4"]["icmp_type"] = buf[0]
        pkt_details["l4"]["icmp_code"] = buf[1]
        pkt_details["l4"]["checksum"]  = (buf[2] << 8) + buf[3]        

    else:
        
        pkt_details["l4"] = False


def checksum(buf, ip_tot_len):

    chksum = 0;
    
    tmp = 0

    i = 1

    while i < ip_tot_len:

        tmp = (buf[i-1] << 8) + buf[i]        
        chksum += tmp
        i += 2

    while chksum >> 16:
        chksum = (chksum >> 16) + (chksum & 0xFFFF)
    
    chksum =  ~chksum;

    return (chksum & 0xFFFF)


def calculate_l3_checksum(buf, ip_tot_len):
    
    buf[10] = 0
    buf[11] = 0

    chksum = checksum(buf, ip_tot_len)

    print("Calculated IP Checksum: %d" %(chksum))
    print("------------------------------------------------------") 
    
    buf[10] = (chksum & 0xFF00) >> 8
    buf[11] = chksum & 0x00FF


def calculate_l4_checksum(buf, pseudo_header, l4_proto):

    TCP_PROTO  = 6
    UDP_PROTO  = 17
    ICMP_PROTO = 1

    pseudo_header_plus_transport_segment = bytearray()

    pseudo_header_plus_transport_segment.extend(pseudo_header)

    if l4_proto == TCP_PROTO:
        
        buf[16] = 0
        buf[17] = 0
    
    elif l4_proto == UDP_PROTO:
        
        buf[6] = 0
        buf[7] = 0

    elif l4_proto == ICMP_PROTO:
        
        buf[2] = 0
        buf[3] = 0

    pseudo_header_plus_transport_segment.extend(buf)

    pseudo_header_plus_transport_segment_len = len(pseudo_header_plus_transport_segment)

    
    if pseudo_header_plus_transport_segment_len % 2:

        pseudo_header_plus_transport_segment.extend(b'\x00')
        pseudo_header_plus_transport_segment_len += 1


    chksum = checksum(pseudo_header_plus_transport_segment, pseudo_header_plus_transport_segment_len)
        
    print("Calculated Transport Checksum: %d" %(chksum))
    print("------------------------------------------------------")
