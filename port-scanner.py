import sys
import os
import socket

def create_sock():

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(1)
    return s

def scan(sock, ip, port):
   
    target_addr = (ip, port)
 
    # print("Scanning port", target_addr)    
    ret = sock.connect_ex(target_addr)

    if ret == 0:
        print("PORT %d : %s" %(port, "OPEN"))
        data = sock.recv(1024)
        data = data.decode(encoding="utf-8")
        print(data)

    ''' 
    print("PORT %d : %s" %(port, "OPEN" if ret == 0 else "CLOSED"))

    if ret != 0:
        print("Error: %s"%(os.strerror(ret)))
    '''

    # print("--------------------------------------------------")

        
def scan_ports(ip, port_range):

    port_min, port_max = [int(x) for x in port_range.split("-")]
 
    for p in range(port_min, port_max + 1):

        sock = create_sock()
        scan(sock, ip, p)
        sock.close()

def main(ip, port_range):

    scan_ports(ip, port_range)

if __name__ == "__main__":
   
    if len(sys.argv) != 3:
        print("Usage: python3 scanner.py [IP] [PORT]")
        exit(0)

    main(sys.argv[1], sys.argv[2])
