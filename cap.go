package main

import (
	"fmt"
	"io"
	"net"
)

func handleClient(client net.Conn) {
	defer client.Close()

	// Connect to the destination server
	server, err := net.Dial("tcp", "batman_server.com:80")
	if err != nil {
		fmt.Println("Error connecting to destination server:", err)
		return
	}
	defer server.Close()

	// Forward data between client and server
	go io.Copy(server, client)
	io.Copy(client, server)
}

func main() {
	listener, err := net.Listen("tcp", "127.0.0.1:8888") // Use the desired local port
	if err != nil {
		fmt.Println("Error starting proxy server:", err)
		return
	}
	defer listener.Close()

	fmt.Println("Proxy server started on port 8888...")

	for {
		client, err := listener.Accept()
		if err != nil {
			fmt.Println("Error accepting client connection:", err)
			continue
		}
		go handleClient(client)
	}
}
