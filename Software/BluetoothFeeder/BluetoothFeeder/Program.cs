using Plugin.BLE;
using Plugin.BLE.Abstractions.Contracts;

namespace BluetoothFeeder
{
    internal class Program
    {
        static void Main(string[] args)
        {
            Start();
        }


        static async void Start()
        {
            var ble = CrossBluetoothLE.Current;
            var adapter = CrossBluetoothLE.Current.Adapter;

            adapter.DeviceDiscovered += (s, a) =>
            {
                Console.WriteLine($"Device found: {a.Device.Name}");
            };

            await adapter.StartScanningForDevicesAsync();
        }
    }
}