import socket
import threading

def handle_client(client_socket):
    # Connect to the destination server
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.connect(("batman_server.com", 80))

    # Forward data between client and server
    client_thread = threading.Thread(target=forward_data, args=(client_socket, server_socket))
    server_thread = threading.Thread(target=forward_data, args=(server_socket, client_socket))

    client_thread.start()
    server_thread.start()

    client_thread.join()
    server_thread.join()

    client_socket.close()
    server_socket.close()

def forward_data(source, destination):
    data = source.recv(4096)
    while len(data):
        destination.send(data)
        data = source.recv(4096)

def main():
    proxy = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    proxy.bind(("127.0.0.1", 8888))  # Use the desired local port
    proxy.listen(5)

    print("Proxy server started on port 8888...")

    while True:
        client_socket, addr = proxy.accept()
        print(f"Accepted connection from {addr[0]}:{addr[1]}")
        client_handler = threading.Thread(target=handle_client, args=(client_socket,))
        client_handler.start()

if __name__ == "__main__":
    main()
