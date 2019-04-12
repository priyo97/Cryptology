'''
Description: This program checks whether a given IP belongs to a given subnet.
USAGE: python3 subnet.py [IP_ADDRESS] [SUBNET_IN_CIDR_FORM]
e.g.-   python3 subnet.py 192.168.70.10 192.168.70.0/24
        python3 subnet.py 172.142.3.2 172.1.0.0/10
'''
import sys

def inet_aton_v4(ipstr):

    ipparts = [ int(x) for x in ipstr.split('.') ]

    return (ipparts[0] << 24) | (ipparts[1] << 16)  | (ipparts[2] << 8) | ipparts[3]
    
def get_int_sub_n_netmask (subnet_cidr):
    
    subnet_str, plen = subnet_cidr.split('/')

    plen = int(plen)

    netmask = 0 

    for i in range(32-plen,32):
        
        netmask |= (1 << i)
       
    subnet_n = inet_aton_v4(subnet_str) 
       
    return subnet_n, netmask



def ipaddr_in_subnet(ip, subnet, mask):
    
    return (ip & mask) == subnet



def main(argc, argv):

    if argc < 3:
    
        print("USAGE: {} [IP] [SUBNET]".format(argv[0]))

    ip_str   = argv[1];

    subnet_str = argv[2];

    ip_n = inet_aton_v4(ip_str)

    subnet_n, netmask = get_int_sub_n_netmask(subnet_str)

    print("{0} is {2}in the subnet {1}".format( ip_str, 
                                                subnet_str, 
                                                "not " if not ipaddr_in_subnet(ip_n, subnet_n, netmask) else ""))
    
        


if __name__ == "__main__":

    main(len(sys.argv), sys.argv)


