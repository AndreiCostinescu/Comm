using Comm.communication;
using Comm.data;
using Comm.socket;
using System;
using System.Diagnostics;
using System.Net;
using System.Threading;
using System.Threading.Tasks;

namespace Comm {
    static class TestConsoleImage {
        public static void ConsoleWriteImage(BitmapModel image) {
            Func<BitmapColor, int> ToConsoleColor = c => {
                int index = (c.r > 128 | c.g > 128 | c.b > 128) ? 8 : 0;
                index |= (c.r > 64) ? 4 : 0;
                index |= (c.g > 64) ? 2 : 0;
                index |= (c.b > 64) ? 1 : 0;
                return index;
            };
            for (int i = 0; i < image.height; i++) {
                for (int j = 0; j < image.width; j++) {
                    Console.ForegroundColor = (ConsoleColor)ToConsoleColor(image.GetPixel(i, j));
                    Console.Write("██");
                }
                /*
                Console.BackgroundColor = ConsoleColor.Black;
                Console.Write("    ");

                for (int j = 0; j < resSize.Width; j++) {
                    Console.ForegroundColor = (ConsoleColor)ToConsoleColor(bmpMax.GetPixel(j * 2, i * 2));
                    Console.BackgroundColor = (ConsoleColor)ToConsoleColor(bmpMax.GetPixel(j * 2, i * 2 + 1));
                    Console.Write("▀");

                    Console.ForegroundColor = (ConsoleColor)ToConsoleColor(bmpMax.GetPixel(j * 2 + 1, i * 2));
                    Console.BackgroundColor = (ConsoleColor)ToConsoleColor(bmpMax.GetPixel(j * 2 + 1, i * 2 + 1));
                    Console.Write("▀");
                }
                //*/
                Console.WriteLine();
            }
            Console.ForegroundColor = ConsoleColor.White;
        }

        public static void test() {
            int width = 20, height = 20;
            BitmapModel x = new BitmapModel(height, width, BitmapFormat.RGB);
            x.Fill(255, 255, 0);
            Console.WindowWidth = 2 * width;
            Console.WindowHeight = 2 * height;
            ConsoleWriteImage(x);
        }
    }

    static class TestEndianness {
        public static void test() {
            Console.WriteLine("Am I big endian? " + utils.NetworkData.isBigEndian());
            Console.WriteLine(sizeof(byte) + " " + sizeof(char) + " " + sizeof(short) + " " + sizeof(int) + " " + sizeof(long));

            ushort t = 48585;
            Console.WriteLine(t + ", " + (ushort)IPAddress.NetworkToHostOrder((short)t) + ", " + (ushort)IPAddress.HostToNetworkOrder((short)t));

            byte[] buffer = null;
            int bufferSize = 0;

            long x1 = (long)(((ulong)1 << 63) - 1), x2 = 1 << 63, resLL;
            Console.WriteLine(x1);
            Console.WriteLine(x2);
            for (int factor = 2; factor <= 15; factor++) {
                long prevYl = -1;
                for (long yL = 0; yL < x1 && yL >= 0; yL++, yL *= factor) {
                    if (prevYl == yL) {
                        break;
                    }
                    Console.WriteLine("yL = " + yL);
                    socket.Utils.prepareBuffer(ref buffer, ref bufferSize, sizeof(long));
                    utils.NetworkData.longLongToNetworkBytes(buffer, 0, yL);
                    resLL = utils.NetworkData.networkBytesToLongLong(buffer, 0);
                    if (resLL != yL) {
                        Console.WriteLine("Error at y = " + yL + ", res = " + resLL);
                    }
                    Debug.Assert(resLL == yL);
                    prevYl = yL;
                }
            }

            socket.Utils.prepareBuffer(ref buffer, ref bufferSize, sizeof(long));
            utils.NetworkData.longLongToNetworkBytes(buffer, 0, x2);
            Debug.Assert(utils.NetworkData.networkBytesToLongLong(buffer, 0) == x2);
            utils.NetworkData.longLongToNetworkBytes(buffer, 0, x1);
            Debug.Assert(utils.NetworkData.networkBytesToLongLong(buffer, 0) == x1);

            int res = 0;
            for (int yI = 0; yI < 1 + 20; yI++) {
                socket.Utils.prepareBuffer(ref buffer, ref bufferSize, sizeof(int));
                utils.NetworkData.intToNetworkBytes(buffer, 0, yI);
                res = utils.NetworkData.networkBytesToInt(buffer, 0);
                if (res != yI) {
                    Console.WriteLine("Error at y = " + yI + ", res = " + res);
                }
                Debug.Assert(res == yI);
            }
            int y = 1 << 31;
            Console.WriteLine(y);
            socket.Utils.prepareBuffer(ref buffer, ref bufferSize, sizeof(int));
            utils.NetworkData.intToNetworkBytes(buffer, 0, y);
            Debug.Assert(utils.NetworkData.networkBytesToInt(buffer, 0) == y);

            double resD;
            for (int factor = 2; factor <= 5; factor++) {
                for (double z = 1.0; z > 0; z /= factor) {
                    Console.WriteLine(z);
                    socket.Utils.prepareBuffer(ref buffer, ref bufferSize, sizeof(double));
                    utils.NetworkData.doubleToNetworkBytes(buffer, 0, z);
                    resD = utils.NetworkData.networkBytesToDouble(buffer, 0);
                    if (resD != z) {
                        Console.WriteLine("Error at z = " + z + ", res = " + resD);
                    }
                    Debug.Assert(resD == z);
                }
            }
        }
    }

    static class TestStrcmp {
        public static void test() {
            Debug.Assert(Int32.Parse("8348\0") == 8348);
            Debug.Assert(utils.Utils.strcmp("asd", "asd") == 0);
            Debug.Assert(utils.Utils.strcmp("asd", "asd\0") == 0);
            Debug.Assert(utils.Utils.strcmp("asd\0", "asd") == 0);
            Debug.Assert(utils.Utils.strcmp("asd\0", "asd\0") == 0);
            Debug.Assert(utils.Utils.strcmp("asd\0", "asd" + (char)1) == -1);
            Debug.Assert(utils.Utils.strcmp("asd" + (char)2, "asd") == 2);
            Debug.Assert(utils.Utils.strcmp("asd" + (char)2, "asd" + (char)1) == 1);
            Debug.Assert(utils.Utils.strcmp("bsd", "asd") == 1);
            Debug.Assert(utils.Utils.strcmp("asd", "bsd") == -1);
        }
    }

    static class TestSocketPartner {
        public static void test() {
            // string ip = "10.151.12.155";
            string ip = "131.159.61.57";
            SocketPartner p = new SocketPartner(ip, 8348, false);
            IPAddress[] ipAddrs = Dns.GetHostAddresses(ip);

            foreach (IPAddress localIPAddr in ipAddrs) {
                Console.WriteLine(localIPAddr.ToString() + ": " + localIPAddr.AddressFamily.ToString());
            }

            IPAddress ipAddr = ipAddrs[0];
            Socket s = new Socket(SocketType.TCP, p);
        }
    }

    static class TestTCPServerTimeoutAcceptConnections {
        public static void testTCPServerTimeoutAcceptConnections() {
            TCPServer s = new TCPServer(8348);
            // s.listen();
            int i = 0;
            while (i++ < 100) {
                Communication comm = s.acceptCommunication();
                if (comm != null) {
                    Console.WriteLine("Found commection!");
                    break;
                } else {
                    Console.WriteLine("No incomming connection!");
                }
            }
        }
    }

    static class TestClient {
        static bool quit = false;
        public static void test() {
            try {
                string x = "";
                while (!(x == "q" || x == "quit")) {
                    Client client = new Client("127.0.0.1", 8348);
                    // Client client = new Client("10.151.11.202", 8348);
                    // Client client = new Client("10.151.12.155", 8348);
                    quit = false;
                    Task t = Task.Run(() => {
                        client.main();
                        quit = true;
                    });
                    while (!quit) {
                        x = Console.ReadLine();
                        if (x == "r" || x == "q" || x == "quit") {
                            client.stop();
                            quit = true;
                        }
                    }
                    t.Wait();
                    Console.WriteLine("Done... before Client destructor!");
                    client = null;
                }
            } catch (Exception e) {
                Console.WriteLine("Caught error/exception: " + e.ToString());
            }
        }
    }

    static class TestTCPServer {
        static readonly int port = 8400;

        static void server() {
            Thread thread = Thread.CurrentThread;
            thread.Name = "Thread 2";
            string x = thread.Name;
            Communication p;
            StatusData status = new StatusData();
            TCPServer t = new TCPServer(port);
            Console.WriteLine(x + ": Created tcp server");

            // t.listen();
            Console.WriteLine(x + ": listening for connections...");

            while (true) {
                Console.WriteLine(x + ": In while loop, waiting for connection...");
                p = t.acceptCommunication();
                if (p != null) {
                    Console.WriteLine(x + ": Created partner communication with " + p.getPartnerString(SocketType.TCP));
                    Debug.Assert(p.recvData(SocketType.TCP, status, false, false, -1, true));
                    Console.WriteLine(x + ": Received: \"" + status.getData() + "\" from partner");
                    Debug.Assert(utils.Utils.strcmp(status.getData(), "Ce faci?") == 0);
                    status.setData("Bine!");
                    Debug.Assert(p.transmitData(SocketType.TCP, status, false, false, -1, true));
                    break;
                }
            }

            p.cleanup();
        }

        static void test_1() {
            Thread.CurrentThread.Name = "Thread 1";
            Thread.Sleep(7000);

            string x = Thread.CurrentThread.Name;
            StatusData status = new StatusData();
            Communication p = new Communication();
            p.createSocket(SocketType.TCP, new SocketPartner("127.0.0.1", port, false));
            Console.WriteLine(x + ": Created partner communication");
            status.setData("Ce faci?");
            Console.WriteLine(x + ": Created status data");
            Debug.Assert(p.transmitData(SocketType.TCP, status, false, false, -1, true));
            Console.WriteLine(x + ": Sent status data");
            Debug.Assert(p.recvData(SocketType.TCP, status, false, false, -1, true));
            Console.WriteLine(x + ": Received status data");
            Console.WriteLine(x + ": Received: \"" + status.getData() + "\" from partner");
            Debug.Assert(utils.Utils.strcmp(status.getData(), "Bine!") == 0);
        }

        public static void test() {
            Console.WriteLine("Hello World!");

            Task t = Task.Run(() => {
                server();
            });
            test_1();
            t.Wait();
        }
    }

    static class TestTCPServerUDPClient {
        static readonly int tcpPort = 8000;

        static void tcpServer() {
            TCPServer server = new TCPServer(tcpPort);
            // server.listen();
            Communication partner;
            while (true) {
                partner = server.acceptCommunication();
                if (partner != null) {
                    Console.WriteLine("Accepted connection");
                    break;
                }
            }

            StatusData s = new StatusData();
            while (!partner.recvData(SocketType.TCP, s, false, false, 0, false)) { }
            Debug.Assert(s.getData() != null);
            int udpPort = Int32.Parse(s.getData());
            Console.WriteLine("Received udp port to communicate on: " + udpPort);
            string udpIP = partner.getPartner(SocketType.TCP).getIP();
            Debug.Assert(udpIP == partner.getPartnerString(SocketType.TCP).Split(':')[0]);
            SocketPartner p = new SocketPartner(udpIP, udpPort, false);
            partner.createSocket(SocketType.UDP, p, -1);
            Console.WriteLine("UDP Client initialized");
            Console.WriteLine("Client: UDP sendTo: " + partner.getPartnerString(SocketType.UDP));
            Console.WriteLine("Client: UDP myself: " + partner.getMyAddressString(SocketType.UDP));
            s.setData("Hello!");
            partner.transmitData(SocketType.UDP, s, false, false, 0, false);
            Console.WriteLine("Client: UDP sendTo: " + partner.getPartnerString(SocketType.UDP));
            Console.WriteLine("Client: UDP myself: " + partner.getMyAddressString(SocketType.UDP));
            s.setData(partner.getMyAddressString(SocketType.UDP));
            partner.transmitData(SocketType.UDP, s, false, false, 0, false);
            Console.WriteLine("Sent my address");
            Console.WriteLine("UDP Server finished normally");
        }

        static void tcpClient() {
            Thread.Sleep(1000);

            Communication p = new Communication();
            p.createSocket(SocketType.TCP, new SocketPartner("127.0.0.1", tcpPort, false));
            p.createSocket(SocketType.UDP, null, 0);
            int udpPort = p.getMyself(SocketType.UDP).getPort();
            Console.WriteLine("Initialized client!");
            StatusData s = new StatusData();
            s.setData("" + udpPort);
            Debug.Assert(p.transmitData(SocketType.TCP, s, false, false));
            Console.WriteLine("Sent udp port: " + udpPort);
            Thread.Sleep(2000);

            Console.WriteLine();
            Console.WriteLine("Server: UDP sendTo: " + p.getPartnerString(SocketType.UDP));
            Console.WriteLine("Server: UDP myself: " + p.getMyAddressString(SocketType.UDP));
            p.recvData(SocketType.UDP, s, false, false, 0, false);
            Debug.Assert(utils.Utils.strcmp(s.getData(), "Hello!") == 0);
            Console.WriteLine("Got data: " + s.getData() + "; expected: \"Hello!\"");
            Console.WriteLine("Server: UDP sendTo: " + p.getPartnerString(SocketType.UDP));
            Console.WriteLine("Server: UDP myself: " + p.getMyAddressString(SocketType.UDP));
            p.recvData(SocketType.UDP, s, false, false, 0, false);
            Console.WriteLine("Got data: " + s.getData() + "; expected: " + p.getPartnerString(SocketType.UDP));
            // Only compare the ports!
            Debug.Assert(utils.Utils.strcmp(s.getData().Split(':')[1], p.getPartnerString(SocketType.UDP).Split(':')[1]) == 0);
            Console.WriteLine("Receive start message");

            Console.WriteLine("Test finished normally");
        }

        public static void test() {
            Console.WriteLine("Hello World!");

            Task t = Task.Run(() => {
                tcpServer();
            });
            tcpClient();
            t.Wait();
        }
    }

    static class TestBothCommunication {
        static readonly int portTCP = 8401;
        static readonly int portUDP = 8402;

        static void server_1() {
            Thread.CurrentThread.Name = "Thread 2";
            string x = Thread.CurrentThread.Name;
            Communication p;
            StatusData status = new StatusData();
            TCPServer t = new TCPServer(portTCP);
            Console.WriteLine(x + ": Created tcp server");

            // t.listen();
            Console.WriteLine(x + ": listening for connections...");

            while (true) {
                Console.WriteLine(x + ": In while loop, waiting for connection...");
                p = t.acceptCommunication();
                if (p != null) {
                    Console.WriteLine(x + ": Created partner communication with " + p.getPartnerString(SocketType.TCP));
                    p.setSocketTimeouts(SocketType.TCP, 2000, 50);
                    break;
                }
            }

            Console.WriteLine(x + ": Starting receive tcp status data...");
            Debug.Assert(p.recvData(SocketType.TCP, status, false, false, -1, true));
            Console.WriteLine(x + ": Received: \"" + status.getData() + "\" from partner");

            status.setData("start");
            Console.WriteLine(x + ": Starting start status data...");
            Debug.Assert(p.transmitData(SocketType.TCP, status, false, false, -1, true));

            Thread.Sleep(3000);

            p.createSocket(SocketType.UDP, null, portUDP, 2000, 50);
            string message;
            while (true) {
                Debug.Assert(p.recvData(SocketType.UDP, status, false, false) || p.getErrorCode() == -1);
                if (p.getErrorCode() == -1) {
                    continue;
                }
                message = status.getData();
                if (message != null) {
                    Console.WriteLine(x + ": Received on UDP \"" + message + "\"");
                    string[] split = message.Split(' ');
                    if (split.Length == 2 && Int32.Parse(split[1]) >= 96) {
                        break;
                    }
                }
            }

            status.setData("stop");
            Debug.Assert(p.transmitData(SocketType.TCP, status, false, false));

            t.cleanup();
            p.cleanup();

            Console.WriteLine(x + ": server finished normally!");
        }

        static void test_1() {
            Thread.Sleep(2000);
            if (Thread.CurrentThread.Name == "") {
                Thread.CurrentThread.Name = "Thread 1";
            }
            string x = Thread.CurrentThread.Name;
            StatusData tcpStatus = new StatusData(), udpStatus = new StatusData();
            SocketPartner partnerTCP = new SocketPartner("127.0.0.1", portTCP, false);
            SocketPartner partnerUDP = new SocketPartner("127.0.0.1", portUDP, false);
            Communication p = new Communication();

            p.createSocket(SocketType.TCP, partnerTCP, -1, 2000, 50);
            Console.WriteLine(x + ": Created TCP partner communication");
            p.createSocket(SocketType.UDP, partnerUDP, -1, 2000, 50);
            Console.WriteLine(x + ": Created TCP & UDP partner communication");

            tcpStatus.setData("Robot 1");
            Console.WriteLine(x + ": Created tcpStatus data");
            Debug.Assert(p.transmitData(SocketType.TCP, tcpStatus, false, false, -1, true));
            Console.WriteLine(x + ": Sent tcpStatus data");
            Debug.Assert(p.recvData(SocketType.TCP, tcpStatus, false, false, -1, true));
            Console.WriteLine(x + ": Received tcpStatus data \"" + tcpStatus.getData() + "\" from partner");
            Debug.Assert(utils.Utils.strcmp(tcpStatus.getData(), "start") == 0);

            Thread.Sleep(3000);

            int id = 0;
            string message;
            while (true) {
                udpStatus.setData("Message " + id++);
                // Console.WriteLine(x + ": set data for udp transmission");
                Debug.Assert(p.transmitData(SocketType.UDP, udpStatus, false, false));
                Console.WriteLine(x + ": sent udp data: " + udpStatus.getData());
                Debug.Assert(p.recvData(SocketType.TCP, tcpStatus, false, false, 0) || p.getErrorCode() == -1);
                // Console.WriteLine(x + ": after tcp listen/recv");
                if (p.getErrorCode() == -1) {
                    continue;
                }
                message = tcpStatus.getData();
                if (message != null) {
                    Console.WriteLine(x + ": received \"" + message + "\" from server");
                    if (utils.Utils.strcmp(message, "stop") == 0) {
                        break;
                    }
                }
            }

            Console.WriteLine(x + ": test_1 finished normally!");
        }

        static void server_2() {
            Thread.CurrentThread.Name = "Thread 2";
            string x = Thread.CurrentThread.Name;
            Communication p;
            StatusData status = new StatusData();
            TCPServer t = new TCPServer(portTCP);
            Console.WriteLine(x + ": Created tcp server");

            // t.listen();
            Console.WriteLine(x + ": listening for connections...");

            while (true) {
                Console.WriteLine(x + ": In while loop, waiting for connection...");
                p = t.acceptCommunication();
                if (p != null) {
                    Console.WriteLine(x + ": Created partner communication with " + p.getPartnerString(SocketType.TCP));
                    p.setSocketTimeouts(SocketType.TCP, 2000, 50);
                    break;
                }
            }

            status.setData("start");
            Console.WriteLine(x + ": Starting start status data...");
            Debug.Assert(p.transmitData(SocketType.TCP, status, false, false, -1, true));

            Console.WriteLine(x + ": Starting receive tcp status data...");
            Debug.Assert(p.recvData(SocketType.TCP, status, false, false, -1, true));
            Console.WriteLine(x + ": Received: \"" + status.getData() + "\" from partner");

            t.cleanup();
            p.cleanup();

            Console.WriteLine(x + ": server finished normally!");
        }

        static void test_2() {
            Thread.Sleep(2000);
            if (Thread.CurrentThread.Name == "") {
                Thread.CurrentThread.Name = "Thread 1";
            }
            string x = Thread.CurrentThread.Name;
            StatusData tcpStatus = new StatusData();
            SocketPartner partner = new SocketPartner("127.0.0.1", portTCP, false);
            Communication p = new Communication();

            p.createSocket(SocketType.TCP, partner, -1, 2000, 50);
            Console.WriteLine(x + ": Created TCP partner communication");

            Debug.Assert(p.recvData(SocketType.TCP, tcpStatus, false, false, -1, true));
            Console.WriteLine(x + ": Received tcpStatus data \"" + tcpStatus.getData() + "\" from partner");

            tcpStatus.setData("Robot 1");
            Console.WriteLine(x + ": Created tcpStatus data");
            Debug.Assert(p.transmitData(SocketType.TCP, tcpStatus, false, false, -1, true));
            Console.WriteLine(x + ": Sent tcpStatus data");

            Console.WriteLine(x + ": test_2 finished normally!");
        }

        public static void test() {
            Console.WriteLine("Hello World!");

            // WSAETIMEDOUT;                                 // <- this should be the error code for timeouts in windows...
            // (errno == EAGAIN) || (errno == EWOULDBLOCK);  // <- this should be the error code for timeouts in linux...

            Task t = Task.Run(() => { server_2(); });
            test_2();
            t.Wait();

            t = Task.Run(() => { server_1(); });
            test_1();
            t.Wait();
        }
    }

    static class TestUDPCommunication {
        static readonly int port = 8400;

        static void server() {
            string x = "Thread 2";
            Communication p = new Communication();
            p.createSocket(SocketType.UDP, null, port, 2000, 50);
            Console.WriteLine("Created partner communication...");
            StatusData status = new StatusData(); ;

            while (true) {
                Console.WriteLine(x + ": In while loop, waiting for connection...");
                Debug.Assert(p.recvData(SocketType.UDP, status, false, false) || p.getErrorCode() == -1);
                if (p.getErrorCode() == -1) {
                    continue;
                }
                if (status.getData() != null) {
                    Console.WriteLine(x + ": Created partner communication with " + p.getPartnerString(SocketType.UDP));
                    Console.WriteLine(x + ": Received: \"" + status.getData() + "\" from partner");
                    Debug.Assert(utils.Utils.strcmp(status.getData(), "Ce faci?") == 0);
                    status.setData("Bine!");
                    Debug.Assert(p.transmitData(SocketType.UDP, status, false, false, -1, true));
                    break;
                }
            }

            Console.WriteLine(x + ": Ending server normally!");
        }

        static void test_1() {
            Thread.Sleep(7000);

            string x = "Thread 1";
            StatusData status = new StatusData();
            Communication p = new Communication();
            p.createSocket(SocketType.UDP, new SocketPartner("127.0.0.1", port, false));
            Console.WriteLine(x + ": Created partner communication");
            status.setData("Ce faci?");
            Console.WriteLine(x + ": Created status data");
            Debug.Assert(p.transmitData(SocketType.UDP, status, false, false, -1, true));
            Console.WriteLine(x + ": Sent status data");
            Debug.Assert(p.recvData(SocketType.UDP, status, false, false, -1, true));
            Console.WriteLine(x + ": Received status data");
            Console.WriteLine(x + ": Received: \"" + status.getData() + "\" from partner");
            Debug.Assert(utils.Utils.strcmp(status.getData(), "Bine!") == 0);

            Console.WriteLine(x + ": Ending test_1 normally!");
        }

        public static void test() {
            Console.WriteLine("Hello World!");

            Task t = Task.Run(() => { server(); });
            test_1();
            t.Wait();
        }
    }

    static class TestSendImageUDP {
        static readonly int port = 8400;
        static readonly bool verbose = false;

        static void sender() {
            Thread.Sleep(1000);

            Communication p = new Communication();
            p.createSocket(SocketType.TCP, new SocketPartner("127.0.0.1", port, false), -1, 2000, 500);
            Console.WriteLine("Initialized sender tcp client!");
            p.createSocket(SocketType.UDP, null, 0, 2000, 500);
            int udpPort = p.getMyself(SocketType.UDP).getPort();
            Console.WriteLine("Initialized sender udp server!");
            StatusData s = new StatusData();
            s.setData("" + udpPort);
            Debug.Assert(p.transmitData(SocketType.TCP, s, false, false));
            Debug.Assert(p.recvData(SocketType.UDP, s, false, false));
            Debug.Assert(utils.Utils.strcmp(s.getData(), "hello") == 0);

            BitmapModel image = new BitmapModel(20, 30, BitmapFormat.RGB); // cv.imread("../../data/Lena.png");
            image.Fill(255, 50, 255);
            ImageData i = new ImageData(image, 1);
            if (!p.transmitData(SocketType.UDP, i, false, false, 0, verbose)) {
                Console.WriteLine("Error: " + p.getErrorCode() + "; " + p.getErrorString());
            } else {
                Console.WriteLine("Sent image data ");
            }

            Console.WriteLine("Sender finished normally");
        }

        static void receiver() {
            TCPServer server = new TCPServer(port);
            // server.listen();
            Communication comm;
            while (true) {
                comm = server.acceptCommunication();
                if (comm != null) {
                    Console.WriteLine("Accepted connection");
                    break;
                }
            }
            comm.setSocketTimeouts(SocketType.TCP, 2000, 500);

            StatusData s = new StatusData();
            while (!comm.recvData(SocketType.TCP, s, false, false)) { }
            Debug.Assert(s.getData() != null);
            SocketPartner p = new SocketPartner(comm.getPartner(SocketType.TCP).getIP(), Int32.Parse(s.getData()), false);
            Console.WriteLine("Connecting to udp: " + p.getPartnerString());
            comm.createSocket(SocketType.UDP, p, -1, 2000, 500);

            s.setData("hello");
            Debug.Assert(comm.transmitData(SocketType.UDP, s, false, false));

            ImageData i = new ImageData();
            if (!comm.recvData(SocketType.UDP, i, false, false, 0, verbose)) {
                Console.WriteLine("Error: " + comm.getErrorCode() + "; " + comm.getErrorString());
            } else if (i.isImageDeserialized()) {
                BitmapModel image = i.getImage();
                TestConsoleImage.ConsoleWriteImage(image);
                /*
                if (!i.getImage().empty()) {
                    imshow("Recv Image", i.getImage());
                    cv.waitKey();
                } else {
                    Console.WriteLine("Error: received image is empty!");
                }
                //*/
            } else {
                Console.WriteLine("Error: deserializing image...");
            }

            Console.WriteLine("Receiver finished normally!");
        }

        public static void test() {
            Console.WriteLine("Hello World!");

            Task t = Task.Run(() => { sender(); });
            receiver();
            t.Wait();
        }
    }

    public static class Tester {
        static void Main(string[] args) {
            Console.WriteLine("Hello World!");

            TestClient.test();
            // TestTCPServerUDPClient.test();
            // TestTCPServer.test();
            // TestBothCommunication.test();
            // TestUDPCommunication.test();
            // TestSendImageUDP.test();

            Console.WriteLine("Finishing test(s)!");
        }
    }
}
