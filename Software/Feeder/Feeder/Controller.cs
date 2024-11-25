using System;
using System.Net;

namespace Feeder
{
    public class Controller
    {
        IPEndPoint endPoint;
        public Controller(IPAddress address, int port)
        {
            endPoint = new IPEndPoint(address, port);
        }

        public IPEndPoint GetEndPoint()
        {
            return endPoint;
        }
    }
}