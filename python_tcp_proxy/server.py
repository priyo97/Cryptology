'''
Reason for using timeout:
    The socket list for p2s and c2p grows dynamically. Initially the p2s list was containing only the server_socket
    which sends it into continuous waiting if timeout is not used. In the c2p and p2s list, there is only the server_socket, 
    because of which both went into waiting state. As soon as first conn came, the c2p function went for processing while 
    the p2s function ignored it and went to the next interation with the same p2s list containing only the server_socket. 
    c2p function did the processing and added a new proxy_to_server socket on the p2s list, but unfortunately the select 
    function was in waiting state with an outdated list, because of which the indefinite wait was there.
'''
import sys
import time
import socket
import signal
import select
import threading

MAX_NCLIENTS = 5

c2p_sock_list = []
c2p_addr_list = []

p2s_sock_list  = []
sock_pair_list = [(-1, -1)]

def sig_handler(*args):
    for sock in c2p_sock_list:
        sock.close()
    
    for sock in p2s_sock_list:
        sock.close()

    exit(1)    

def get_client_id(s):
    return c2p_sock_list.index(s)

def get_corr_sock(sock, sock_side):
  
    sock_side_map = {"c2p": 0, "p2s": 1} 

    sock_side_idx = sock_side_map[sock_side]

    for idx, sock_pair in enumerate(sock_pair_list):
        if sock == sock_pair[sock_side_idx]:
            return sock_pair[sock_side_idx - 1], idx
            

def serve_client(c2p, addr, tlock):
   
    print("(%s, %s) >> "%(addr[0], addr[1]))
    
    data = c2p.recv(4096)

    p2s, idx = get_corr_sock(c2p, "c2p")

    if not data:
        print("Bye")
        tlock.acquire()
        c2p.close()
        p2s.close()

        del sock_pair_list[idx]
        del c2p_sock_list[idx]
        del c2p_addr_list[idx] 
        del p2s_sock_list[idx]

        tlock.release()
        return

    #print(data.decode(encoding="utf-8"))   
    p2s.sendall(data)

    # print("(%s, %s) >> "%(addr[0], addr[1]) + data.decode(encoding="utf-8"))

def serve_proxy(p2s):
     
    data = p2s.recv(4096)
    c2p, idx = get_corr_sock(p2s, "p2s")
    
    if not data:
        print("p2s socket close") 
        return
    #    print(data.decode(encoding="utf-8"))
    print("p2s to c2p")        
    c2p.sendall(data)

 
def serve_server(server_socket, remote_host_ip, remote_host_port):

    c2p, addr = server_socket.accept()

    p2s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    p2s.connect((remote_host_ip, remote_host_port))

    sock_pair_list.append((c2p, p2s))
    c2p_sock_list.append(c2p)
    p2s_sock_list.append(p2s)
    c2p_addr_list.append(addr)

    print("Connection from: (%s, %s) | Connection to: %s" %(addr[0], addr[1], remote_host_ip))

def serve_c2p(server_socket, remote_host_ip, remote_host_port, tlock):

    while True:
        time.sleep(1)
        tlock.acquire()
        read_list, *extra = select.select(c2p_sock_list, [], [], 1)
        tlock.release()
        for s in read_list:
            if s == server_socket:
                serve_server(s, remote_host_ip, remote_host_port)
            else:
                client_idx  = get_client_id(s)
                client_addr = c2p_addr_list[client_idx] 
                serve_client(s, client_addr, tlock)


def serve_p2s(server_socket, tlock):

    while True:
        time.sleep(1)
        tlock.acquire()
        read_list, *extra = select.select(p2s_sock_list, [], [], 1)
        tlock.release()
        for s in read_list:
            if s != server_socket:
                serve_proxy(s)

 
def main(host_ip, host_port):

    signal.signal(signal.SIGINT, sig_handler)

    remote_host_name = "www.youtube.com"
    remote_host_port = 443

    #remote_host_ip = socket.gethostbyname(remote_host_name)
    remote_host_ip = "172.217.31.196"

    tlock = threading.Lock()

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    addr = (host_ip, int(host_port))
    server_socket.bind(addr)
    server_socket.listen(MAX_NCLIENTS)

    print("Server listening on (%s, %s)" %(host_ip, host_port))

    c2p_sock_list.append(server_socket)
    p2s_sock_list.append(server_socket)
    c2p_addr_list.append(addr)

    t1 = threading.Thread(target=serve_c2p, args=(server_socket, remote_host_ip, remote_host_port, tlock)) 
    t2 = threading.Thread(target=serve_p2s, args=(server_socket, tlock)) 
   
    t1.start()
    t2.start()
    

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python server.py [HOST_IP] [HOST_PORT]")
        exit(1)
    try:
        main(sys.argv[1], sys.argv[2])
    except Exception as e:
        print("Error:")
        sig_handler()
