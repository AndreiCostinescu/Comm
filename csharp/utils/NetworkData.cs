using System;
using System.Net;

namespace DesignAIRobotics.Comm.utils {
    class NetworkData {
        private static bool bigEndian = !BitConverter.IsLittleEndian;
        public static ulong shortSize = sizeof(short);
        public static ulong intSize = sizeof(int);
        public static ulong longSize = sizeof(long);
        public static ulong floatSize = sizeof(float);
        public static ulong doubleSize = sizeof(double);

        static public bool isBigEndian() {
            return bigEndian;
        }

        static public bool isLittleEndian() {
            return !isBigEndian();
        }

        public static void shortToNetworkBytes(byte[] buffer, int start, short value) {
            Utils.memcpy(buffer, (ulong) start, BitConverter.GetBytes(IPAddress.HostToNetworkOrder(value)), 0, shortSize);
        }

        public static short networkBytesToShort(byte[] buffer, int start) {
            return IPAddress.NetworkToHostOrder(BitConverter.ToInt16(buffer, start));
        }

        public static void intToNetworkBytes(byte[] buffer, int start, int value) {
            Utils.memcpy(buffer, (ulong) start, BitConverter.GetBytes(IPAddress.HostToNetworkOrder(value)), 0, intSize);
        }

        public static int networkBytesToInt(byte[] buffer, int start) {
            return IPAddress.NetworkToHostOrder(BitConverter.ToInt32(buffer, start));
        }

        public static void longLongToNetworkBytes(byte[] buffer, int start, long value) {
            Utils.memcpy(buffer, (ulong) start, BitConverter.GetBytes(IPAddress.HostToNetworkOrder(value)), 0, longSize);
        }

        public static long networkBytesToLongLong(byte[] buffer, int start) {
            return IPAddress.NetworkToHostOrder(BitConverter.ToInt64(buffer, start));
        }

        public static void floatToNetworkBytes(byte[] buffer, int start, double value) {
            Utils.memcpy(buffer, (ulong) start, BitConverter.GetBytes(value), 0, floatSize);
        }

        public static float networkBytesToFloat(byte[] buffer, int start) {
            return BitConverter.ToSingle(buffer, start);
        }

        public static void doubleToNetworkBytes(byte[] buffer, int start, double value) {
            Utils.memcpy(buffer, (ulong) start, BitConverter.GetBytes(value), 0, doubleSize);
        }

        public static double networkBytesToDouble(byte[] buffer, int start) {
            return BitConverter.ToDouble(buffer, start);
        }
    }
}
