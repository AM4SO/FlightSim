using System.Net;
using System.Net.Sockets;
using System.Text;

namespace HelpTesting
{
    internal class Program
    {
        static UdpClient udpClient;
        static void Main(string[] args)
        {
            udpClient = new UdpClient();

            byte[] message = Encoding.ASCII.GetBytes("Hello, this is a message");

            while (true)
            {
                Console.WriteLine("Sending...");
                udpClient.Send(message, message.Length, new IPEndPoint(IPAddress.Parse("224.0.2.60"), 4446));

                Thread.Sleep(1000);
            }
        }
    }
}