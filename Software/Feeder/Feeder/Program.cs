using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace Feeder
{
    internal class Program
    {
        static UdpClient udpClient;
        static void Main(string[] args)
        {
            udpClient = new UdpClient(25575);

            Console.WriteLine("Listening...");
            startListen();
        }

        static void startListen()
        {
            while (true)
            {
                IPEndPoint endPoint = new IPEndPoint(IPAddress.Any, 0);
                byte[] results = udpClient.Receive(ref endPoint);

                Console.Write("Received packets from end point: ");
                Console.WriteLine(endPoint.Address.ToString() + " " + endPoint.Port);
                Console.WriteLine("Received: " + Encoding.ASCII.GetString(results));
            }
        }
    }
}