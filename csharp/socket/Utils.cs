using System.Net;

namespace Comm.socket {
    class Utils {
        public static readonly int SOCKET_ACCEPT_TIMEOUT_SECONDS = 1;

        public static readonly ulong CLIENT_MAX_MESSAGE_BYTES = 65500;

        public static readonly int SOCKET_BUFFER_RECV_SIZE = 4 * 1024 * 1024;

        public static readonly int SOCKET_BUFFER_SEND_SIZE = 4 * 1024 * 1024;

        static string getAddressIP(IPEndPoint address) {
            return address.Address.ToString();
        }

        static int getAddressPort(IPEndPoint address) {
            return address.Port;
        }

        static string getStringAddress(IPEndPoint address) {
            return address.ToString();
        }

        void prepareBuffer(ref byte[] buffer, int desiredLength) {
            // cout << "Initialize new buffer!" << endl;
            buffer = new byte[desiredLength];
            // cout << "Initialized new buffer (" << bufferLength << ")!" << endl;
            utils.Utils.memset(buffer, 0, 0, (ulong) desiredLength * sizeof(byte));
        }

        public static void prepareBuffer(ref byte[] buffer, ref int bufferLength, int desiredLength) {
            if (bufferLength < desiredLength) {
                // cout << "Initialize new buffer!" << endl;
                buffer = new byte[desiredLength];
                bufferLength = desiredLength;
                // cout << "Initialized new buffer (" << bufferLength << ")!" << endl;
            }
            utils.Utils.memset(buffer, 0, 0, (ulong)bufferLength * sizeof(byte));
        }

        public static void prepareBuffer(ref byte[] buffer, ref ulong bufferLength, ulong desiredLength) {
            if (bufferLength < desiredLength) {
                // cout << "Initialize new buffer!" << endl;
                buffer = new byte[desiredLength];
                bufferLength = desiredLength;
                // cout << "Initialized new buffer (" << bufferLength << ")!" << endl;
                utils.Utils.memset(buffer, 0, 0, bufferLength * sizeof(byte));
            }
        }

        public static bool comparePartners(IPEndPoint p1, IPEndPoint p2) {
            return p1.Address.ToString() == p2.Address.ToString() && p1.Port == p2.Port;
        }
    }
}
