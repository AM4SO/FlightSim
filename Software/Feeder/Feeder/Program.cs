using Microsoft.VisualBasic;
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
            udpClient = new UdpClient(4446);
            udpClient.JoinMulticastGroup(IPAddress.Parse("224.0.2.60")); // 4446

            Console.WriteLine("Listening...");
            startListen();
        }

        static void startListen()
        {
            while (true)
            {
                IPEndPoint endPoint = new IPEndPoint(IPAddress.Any, 0);
                byte[] results = udpClient.Receive(ref endPoint);

                Console.Write(" " + endPoint.Address.ToString() + ":" + endPoint.Port + " @ " + DateTime.Now.TimeOfDay.ToString().Substring(0,8) + ": ");
                Console.WriteLine(Encoding.ASCII.GetString(results));
            }
        }
    }
}