import sys
import socket
import signal
import threading

sock_list = []

def sig_handler(*args):
    for sock in sock_list:
        sock.close()
    exit(1)    

def main(server_ip, server_port):

    signal.signal(signal.SIGINT, sig_handler)

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
   
    server_addr = (server_ip, int(server_port))
    client_socket.connect(server_addr)

    print("Connected to server on (%s, %s)" %(server_ip, server_port))

    sock_list.append(client_socket)

    while True:
        message = input(">> ") 
        client_socket.sendall(message.encode(encoding="utf-8"))

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python client.py [SERVER_IP] [SERVER_PORT]")
        exit(1)

    main(sys.argv[1], sys.argv[2])
