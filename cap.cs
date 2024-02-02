using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

class ProxyServer
{
    static async Task Main(string[] args)
    {
        TcpListener listener = new TcpListener(IPAddress.Loopback, 8888); // Use the desired local port
        listener.Start();
        Console.WriteLine("Proxy server started on port 8888...");

        while (true)
        {
            TcpClient client = await listener.AcceptTcpClientAsync();
            _ = HandleClient(client);
        }
    }

    static async Task HandleClient(TcpClient client)
    {
        using (var clientStream = client.GetStream())
        {
            var destinationServer = new TcpClient("batman_server.com", 80);

            using (var serverStream = destinationServer.GetStream())
            {
                await Task.WhenAll(
                    CopyDataAsync(clientStream, serverStream),
                    CopyDataAsync(serverStream, clientStream)
                );
            }
        }

        client.Close();
    }

    static async Task CopyDataAsync(NetworkStream source, NetworkStream destination)
    {
        try
        {
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = await source.ReadAsync(buffer, 0, buffer.Length)) > 0)
            {
                await destination.WriteAsync(buffer, 0, bytesRead);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error: {ex.Message}");
        }
    }
}
