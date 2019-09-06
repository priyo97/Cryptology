import sys
import socket
import signal
import select
import threading

MAX_NCLIENTS = 5

sock_list = []
sock_addr_list = []


def sig_handler(*args):
    for sock in sock_list:
        sock.close()
    exit(1)    

def get_client_id(s):
    return sock_list.index(s)

def serve_client(c, addr):
    
    data = c.recv(4096)

    if not data:
        print("Bye")
        idx = get_client_id(c)
        c.close()
        del sock_list[idx]
        del sock_addr_list[idx] 
        return

    print("(%s, %s) >> "%(addr[0], addr[1]) + data.decode(encoding="utf-8"))
 
def serve_server(server_socket):

    c, addr = server_socket.accept()
    sock_list.append(c)
    sock_addr_list.append(addr)
    print("Connected to: (%s, %s)" %(addr[0], addr[1]))


def serve(server_socket):

    while True: 
        read_list, *extra = select.select(sock_list, [], [])
        for s in read_list:
            if s == server_socket:
                serve_server(s)
            else:
                client_idx  = get_client_id(s)
                client_addr = sock_addr_list[client_idx] 
                serve_client(s, client_addr)

 
def main(host_ip, host_port):

    signal.signal(signal.SIGINT, sig_handler)

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    addr = (host_ip, int(host_port))
    server_socket.bind(addr)
    server_socket.listen(MAX_NCLIENTS)

    print("Server listening on (%s, %s)" %(host_ip, host_port))

    sock_list.append(server_socket)
    sock_addr_list.append(addr)

    t = threading.Thread(target=serve, args={server_socket,})
    t.start()

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python server.py [HOST_IP] [HOST_PORT]")
        exit(1)

    main(sys.argv[1], sys.argv[2])
