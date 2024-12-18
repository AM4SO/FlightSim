using Microsoft.VisualBasic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Net;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Text;
using System.Collections.Generic;
using System.Transactions;
using vJoyInterfaceWrap;

namespace Feeder
{
    internal class Program
    {
        static string MULTICAST_IP = "224.0.2.60";

        // Udp clients for communication with joystick
        static UdpClient udpClient = new UdpClient(4446);
        static UdpClient udpSender = new UdpClient();

        // To keep track of each connected device
        static List<Controller> controllers = new List<Controller>();
        static HashSet<IPAddress> controllerAddresses = new HashSet<IPAddress>();

        // Labels and information about each axis
        static Dictionary<byte, Axis> axes = new Dictionary<byte, Axis>();

        // status variables for pairing sequence with a joystick
        static Controller pairingController = new Controller(IPAddress.Any, 0);
        static BooleanVariable finishedPairing = new BooleanVariable(false);
        static bool startedPairSequence = false;

        static Feeder feeder = new Feeder(1);


        static void Main(string[] args)
        {
            // Add the axes to be used
            axes.Add(1, new Axis(HID_USAGES.HID_USAGE_Y ,"Y"));
            axes.Add(2, new Axis(HID_USAGES.HID_USAGE_X ,"X"));

            // Join multicast group to allow joystick to find us
            udpClient.JoinMulticastGroup(IPAddress.Parse(MULTICAST_IP));

            // Start accepting connections
            Console.WriteLine("Listening...");
            StartListen();
        }
        
        // interprets data from connected controllers, and feeds them
        static void AcceptControllerInput(byte[] inputBytes)
        {
            // Input bytes are assumed to follow format: [axisId, B0, ..., Bn, axisId, B0, ..., Bn, ...]
            int i = 0;
            while(i < inputBytes.Length)
            {
                byte axisId = inputBytes[i];
                Axis axis = axes[axisId];
                i++;
                short value = 0;

                for (int j = 0; j < axis.size; j++)
                {
                    value <<= 8;
                    byte b = inputBytes[i];
                    value += b;
                    i++;
                }

                Console.Write(axis.name + ": " + value.ToString() + "  ");

                feeder.FeedAxis(axis.axisUsage, value);
            }

            Console.WriteLine();
            feeder.flushFeed();
        }

        static void AsyncRepeatUntilCondition(Action action, BooleanVariable condition, int timeout_ms=20)
        {
            new Thread(new ThreadStart(() => {
                while (!condition.Value)
                {
                    action.Invoke();
                    Thread.Sleep(timeout_ms);
                }
                condition.Value = false;
            })).Start();
        }

        static void StartListen()
        {
            while (true)
            {
                IPEndPoint endPoint = new IPEndPoint(IPAddress.Any, 0);
                byte[] results = udpClient.Receive(ref endPoint);
                

                // 0 length messages shouldn't exist. Announce them for debugging purposes
                if (results.Length == 0)
                {
                    Console.WriteLine("0 length message received from: " + endPoint.Address + ":" + endPoint.Port);
                    continue;
                }

                if (results.Length > 1)
                {
                    // If we are in the process of pairing with the sender joystick
                    if ((!controllerAddresses.Contains(endPoint.Address)) ||
                        (startedPairSequence && endPoint.Address.Equals(pairingController.GetEndPoint().Address)))
                    {
                        Console.WriteLine("Received pair acknowledge from joystick");
                        controllerAddresses.Add(endPoint.Address);
                        controllers.Add(pairingController);
                        finishedPairing.Value = true;
                        startedPairSequence = false;
                    }

                    Console.Write(" " + endPoint.Address.ToString() + ":" + endPoint.Port + " @ " + DateTime.Now.TimeOfDay.ToString().Substring(0, 8) + ": ");
                    AcceptControllerInput(results);
                    continue;
                }

                if (results[0] == 10)
                {
                    Console.WriteLine(DateTime.Now.TimeOfDay.ToString());
                    //udpSender.SendAsync(new byte[] { 11 }, 1, pairingController.GetEndPoint());
                    continue;
                }

                if (results[0] == 0 && !startedPairSequence && !controllerAddresses.Contains(endPoint.Address))
                {
                    startedPairSequence = true;

                    Console.Write("Starting pair sequence with controller at address: ");
                    Console.WriteLine(endPoint.Address.ToString() + ":" + endPoint.Port);

                    pairingController = new Controller(endPoint.Address, 25575);

                    AsyncRepeatUntilCondition(() => {
                        Console.WriteLine("Sending message to " + pairingController.GetEndPoint().Address + ":" + pairingController.GetEndPoint().Port);
                        udpClient.SendAsync(new byte[] { 1 }, 1, pairingController.GetEndPoint()); 
                    }, finishedPairing, 10);
                }
                else
                {
                    Console.WriteLine("Length 1 packet received from already existing client");
                }


            }
        }
    }

    class BooleanVariable
    {
        public bool Value { get; set; }
        public BooleanVariable(bool value) { this.Value = value; }
    }

    class Axis
    {
        public string name;
        public int size;
        public HID_USAGES axisUsage;

        public Axis(HID_USAGES axisUsage, string name, int size=2)
        {
            this.axisUsage = axisUsage;
            this.name = name;
            this.size = size;
        }

    }
}