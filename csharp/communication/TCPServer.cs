using Comm.socket;
using System;
using System.Diagnostics;

namespace Comm.communication {
    class TCPServer {
        public TCPServer(int port, int backlog = 5) {
            this.port = port;
            this.backlog = backlog;
            this.socket = null;
            this.initServerSocket();
        }

        ~TCPServer() {
            Console.WriteLine("In TCPServer destructor!");
            this.cleanup();
        }

        public void cleanup() {
            this.socket.cleanup();
        }

        private void listen() {
            this.socket.getSocket().Listen(this.backlog);
        } 

        public Communication acceptCommunication() {
            try {
                System.Net.Sockets.Socket acceptedSocket = this.socket.getSocket().Accept();
                Debug.Assert(acceptedSocket.Blocking == this.socket.getSocket().Blocking);
                acceptedSocket.Blocking = true;
                // The accept() call actually accepts an incoming connection
                Communication comm = new Communication();
                Socket tcpSocket = null;
                this.socket.accept(ref tcpSocket, acceptedSocket);
                comm.setSocket(SocketType.TCP, socket);
                return comm;
            } catch (System.Net.Sockets.SocketException se) {
                int errorCode = se.ErrorCode;
                if (errorCode == 10035) {
                    // Timeout occurred!
                    return null; 
                } else {
                    Console.WriteLine("ErrorCode in acceptCommunication = " + errorCode);
                    this.initServerSocket();
                    return null;
                }
            }
        }

        private void initServerSocket() {
            if (this.socket != null) {
                this.socket.cleanup();
                this.socket.initialize(SocketType.TCP, null, this.port);
            } else {
                this.socket = new Socket(SocketType.TCP, null, this.port);
            }
            this.socket.getSocket().Blocking = false;
            this.listen();
        }

        private int port, backlog;
        private Socket socket;
    };
}
