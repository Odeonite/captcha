import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;

public class cap {
    public static void main(String[] args) {
        try {
            ServerSocket proxySocket = new ServerSocket(8888); // Use the desired local port
            System.out.println("Proxy server started on port 8888...");

            while (true) {
                Socket clientSocket = proxySocket.accept();
                System.out.println("Accepted connection from " + clientSocket.getInetAddress() +
                        ":" + clientSocket.getPort());

                new Thread(() -> handleClient(clientSocket)).start();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static void handleClient(Socket clientSocket) {
        try {
            // Connect to the destination server
            Socket serverSocket = new Socket("batman_server.com", 80);
            System.out.println("Connected to destination server");

            // Forward data between client and server
            Thread clientToServer = new Thread(() -> forwardData(clientSocket, serverSocket));
            Thread serverToClient = new Thread(() -> forwardData(serverSocket, clientSocket));

            clientToServer.start();
            serverToClient.start();

            clientToServer.join();
            serverToClient.join();

            clientSocket.close();
            serverSocket.close();
        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        }
    }

    private static void forwardData(Socket source, Socket destination) {
        try (InputStream sourceInputStream = source.getInputStream();
            OutputStream destinationOutputStream = destination.getOutputStream()) {

            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = sourceInputStream.read(buffer)) != -1) {
                destinationOutputStream.write(buffer, 0, bytesRead);
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
