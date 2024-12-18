﻿using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using vJoyInterfaceWrap;

namespace Feeder
{
    internal class Feeder
    {
        // Declaring one joystick (Device id 1) and a position structure. 
        public vJoy joystick;
        public vJoy.JoystickState iReport;
        public uint id = 1;

        protected Queue<AxisFeeding> toFeed;

        public Feeder(uint id)
        {
            toFeed = new Queue<AxisFeeding>();

            this.id = id;

            // Create one joystick object and a position structure.
            joystick = new vJoy();
            iReport = new vJoy.JoystickState();

            Init();

            joystick.FfbRegisterGenCB(new vJoy.FfbCbFunc((packet, obj) =>
            {
                UInt32 type = 0;
                Int32 size = 0;
                byte[] data = { };
                uint res = joystick.Ffb_h_Packet(packet, ref type, ref size, ref data);

                if (res != 0)
                {
                    Console.WriteLine("Failure with ffb");
                }

                FFBEType[] types = Enum.GetValues<FFBEType>();

                string typeName = ((FFBEType)Enum.ToObject(typeof(FFBEType), type)).ToString();

                Console.WriteLine("FFB effect type: " + typeName);

            }), null);
        }

        public void FeedAxis(HID_USAGES axis, int value)
        {
            toFeed.Enqueue(new AxisFeeding(axis, value));
        }
        public bool flushFeed()
        {
            bool res = true;
            while(toFeed.Count > 0)
            {
                AxisFeeding feeding = toFeed.Dequeue();
                res = res && joystick.SetAxis(feeding.value, id, feeding.axis);
            }

            return res;
        }

        protected void Init()
        {

            if (id <= 0 || id > 16)
            {
                Console.WriteLine("Illegal device ID {0}\nExit!", id);
                return;
            }

            // Get the driver attributes (Vendor ID, Product ID, Version Number)
            if (!joystick.vJoyEnabled())
            {
                Console.WriteLine("vJoy driver not enabled: Failed Getting vJoy attributes.\n");
                return;
            }
            else
                Console.WriteLine("Vendor: {0}\nProduct :{1}\nVersion Number:{2}\n", joystick.GetvJoyManufacturerString(), joystick.GetvJoyProductString(), joystick.GetvJoySerialNumberString());

            // Get the state of the requested device
            VjdStat status = joystick.GetVJDStatus(id);
            switch (status)
            {
                case VjdStat.VJD_STAT_OWN:
                    Console.WriteLine("vJoy Device {0} is already owned by this feeder\n", id);
                    break;
                case VjdStat.VJD_STAT_FREE:
                    Console.WriteLine("vJoy Device {0} is free\n", id);
                    break;
                case VjdStat.VJD_STAT_BUSY:
                    Console.WriteLine("vJoy Device {0} is already owned by another feeder\nCannot continue\n", id);
                    return;
                case VjdStat.VJD_STAT_MISS:
                    Console.WriteLine("vJoy Device {0} is not installed or disabled\nCannot continue\n", id);
                    return;
                default:
                    Console.WriteLine("vJoy Device {0} general error\nCannot continue\n", id);
                    return;
            };

            // Check which axes are supported
            bool AxisX = joystick.GetVJDAxisExist(id, HID_USAGES.HID_USAGE_X);
            bool AxisY = joystick.GetVJDAxisExist(id, HID_USAGES.HID_USAGE_Y);
            bool AxisZ = joystick.GetVJDAxisExist(id, HID_USAGES.HID_USAGE_Z);
            bool AxisRX = joystick.GetVJDAxisExist(id, HID_USAGES.HID_USAGE_RX);
            bool AxisRZ = joystick.GetVJDAxisExist(id, HID_USAGES.HID_USAGE_RZ);
            // Get the number of buttons and POV Hat switchessupported by this vJoy device
            int nButtons = joystick.GetVJDButtonNumber(id);
            int ContPovNumber = joystick.GetVJDContPovNumber(id);
            int DiscPovNumber = joystick.GetVJDDiscPovNumber(id);

            // Print results
            Console.WriteLine("\nvJoy Device {0} capabilities:\n", id);
            Console.WriteLine("Numner of buttons\t\t{0}\n", nButtons);
            Console.WriteLine("Numner of Continuous POVs\t{0}\n", ContPovNumber);
            Console.WriteLine("Numner of Descrete POVs\t\t{0}\n", DiscPovNumber);
            Console.WriteLine("Axis X\t\t{0}\n", AxisX ? "Yes" : "No");
            Console.WriteLine("Axis Y\t\t{0}\n", AxisY ? "Yes" : "No");
            Console.WriteLine("Axis Z\t\t{0}\n", AxisZ ? "Yes" : "No");
            Console.WriteLine("Axis Rx\t\t{0}\n", AxisRX ? "Yes" : "No");
            Console.WriteLine("Axis Rz\t\t{0}\n", AxisRZ ? "Yes" : "No");

            // Test if DLL matches the driver
            UInt32 DllVer = 0, DrvVer = 0;
            bool match = joystick.DriverMatch(ref DllVer, ref DrvVer);
            if (match)
                Console.WriteLine("Version of Driver Matches DLL Version ({0:X})\n", DllVer);
            else
                Console.WriteLine("Version of Driver ({0:X}) does NOT match DLL Version ({1:X})\n", DrvVer, DllVer);


            // Acquire the target
            if ((status == VjdStat.VJD_STAT_OWN) || ((status == VjdStat.VJD_STAT_FREE) && (!joystick.AcquireVJD(id))))
            {
                Console.WriteLine("Failed to acquire vJoy device number {0}.\n", id);
                return;
            }
            else
                Console.WriteLine("Acquired: vJoy device number {0}.\n", id);

            long maxval = 0;
            long minval = 0;

            joystick.GetVJDAxisMax(id, HID_USAGES.HID_USAGE_X, ref maxval);
            joystick.GetVJDAxisMin(id, HID_USAGES.HID_USAGE_X, ref minval);
            Console.WriteLine("Axis min value: " + minval.ToString());
            Console.WriteLine("Axis max value: " + maxval.ToString());

            // Reset this device to default values
            joystick.ResetVJD(id);

        }
    }

    class AxisFeeding
    {
        public HID_USAGES axis;
        public int value;
        public AxisFeeding(HID_USAGES axis, int value)
        {
            this.axis = axis;
            this.value = value;
        }
    }
}
