using Comm.utils;
// using DesignAIRobotics.Utils;
using System;
using System.Diagnostics;
using System.Net;

namespace Comm.socket {
    public class Socket {
        public Socket(SocketType protocol, SocketPartner partner = null, int myPort = 0, int sendTimeout = -1, int recvTimeout = -1) : this() {
            this.initialize(protocol, partner, myPort, sendTimeout, recvTimeout);
        }

        ~Socket() {
            this.cleanup(true);
        }

        public ref System.Net.Sockets.Socket getSocket() {
            return ref this.socket;
        }

        public void cleanup(bool withBuffers = false) {
            Console.WriteLine("Closing socket: " + ((this.myself == null) ? "???" : this.myself.getPartnerString()) +
                " last connected to " + ((this.partner == null) ? "???" : this.partner.getPartnerString()));
            this.close();
            this.partner = null;
            this.myself = null;
            this.isCopy = false;
            this.initialized = false;
            if (withBuffers) {
                this.sendBuffer = null;
                this.recvBuffer = null;
            }
        }

        public void close() {
            if (!this.isCopy && this.socket != null) {
                this.socket.Close();
            }
            this.socket = null;
        }

        public Socket copy() {
            Socket copy = new Socket();
            Debug.Assert(copy.socket == null);
            copy.protocol = this.protocol;
            copy.socket = this.socket;

            copy.partner = this.partner.copy();
            copy.myself = this.myself.copy();

            copy.recvAddress = this.recvAddress;

            if (this.sendBuffer != null) {
                copy.sendBuffer = this.sendBuffer.copy();
            }
            if (this.recvBuffer != null) {
                copy.recvBuffer = this.recvBuffer.copy();
            }

            copy.sendTimeout = this.sendTimeout;
            copy.recvTimeout = this.recvTimeout;

            copy.isCopy = true;
            copy.initialized = true;
            return copy;
        }

        public bool initialize() {
            this.createSocket();

            if (!this.initMyself()) {
                return false;
            }

            if (this.partner != null) {
                if (this.protocol == SocketType.TCP) {
                    Console.WriteLine("Socket Initialization: connecting tcp socket...");
                    try {
                        // this.socket.Connect(this.partner.getPartner());
                        int countElapsedSecondsSinceConnectionAttempt = 0;
                        while (countElapsedSecondsSinceConnectionAttempt <= 20) {
                            IAsyncResult result = this.socket.BeginConnect(this.partner.getPartner(), null, null);
                            bool success = result.AsyncWaitHandle.WaitOne(1000, true);
                            countElapsedSecondsSinceConnectionAttempt += 1;
                            if (!this.socket.Connected) {
                                if (countElapsedSecondsSinceConnectionAttempt >= 20) {
                                    DebugUtils.SetOutput("Can not connect to server!");
                                    throw new System.Net.Sockets.SocketException(10061);
                                } else if (countElapsedSecondsSinceConnectionAttempt == 10) {
                                    DebugUtils.SetOutput("Server may not be online... Try again later?");
                                    Console.WriteLine("Server may not be online... Try again later?");
                                } else if (countElapsedSecondsSinceConnectionAttempt == 5) {
                                    DebugUtils.SetOutput("Server may not be online...");
                                    Console.WriteLine("Server may not be online...");
                                }
                                this.socket.Close();
                                this.createSocket();
                                Debug.Assert(this.initMyself());
                            } else {
                                Debug.Assert(this.socket != null);
                                this.socket.EndConnect(result);
                                break;
                            }
                        }
                    } catch (ArgumentNullException ane) {
                        Console.WriteLine("ArgumentNullException : {0}", ane.ToString());
                        return false;
                    } catch (System.Net.Sockets.SocketException se) {
                        Console.WriteLine("Connection failed due to port and ip problems");
                        if (se.ErrorCode == 10061 || se.ErrorCode == 10051) {
                            // Connection refused
                            throw se;
                        }
                        Console.WriteLine("SocketException : {0}", se.ToString());
                        return false;
                    } catch (Exception e) {
                        Console.WriteLine("Unexpected exception : {0}", e.ToString());
                        return false;
                    }
                    Console.WriteLine("Socket Initialization: connected tcp socket to " + this.partner.getPartnerString());
                }
            }

            _setSocketTimeouts(this.socket, this.sendTimeout, this.recvTimeout);

            // printSocketDetails(this.socket);
            this.initialized = true;
            return true;
        }

        public bool initialize(SocketType _protocol, SocketPartner _partner = null, int _myPort = 0, int _sendTimeout = -1, int _recvTimeout = -1) {
            this.cleanup();
            this.protocol = _protocol;
            this.partner = _partner;
            if (_myPort < 0 || _myPort >= (1 << 16)) {
                _myPort = 0;
            }
            this.myself = new SocketPartner("", _myPort, false);
            this.sendTimeout = _sendTimeout;
            this.recvTimeout = _recvTimeout;
            return this.initialize();
        }

        public void setSocketTimeouts(int sendTimeout = -1, int recvTimeout = -1, bool modifySocket = true) {
            this.sendTimeout = sendTimeout;
            this.recvTimeout = recvTimeout;
            if (modifySocket) {
                _setSocketTimeouts(this.socket, this.sendTimeout, this.recvTimeout);
            }
        }

        public void setOverwritePartner(bool overwrite) {
            if (this.isCopy) {
                Console.WriteLine("Can not change the partner of a copy socket!");
                return;
            }
            if (this.partner == null) {
                this.partner = new SocketPartner();
            }
            this.partner.setOverwrite(overwrite);
        }

        public void setPartner(string partnerIP, int partnerPort, bool overwrite) {
            if (this.isCopy) {
                Console.WriteLine("Can not change the partner of a copy socket!");
                return;
            }
            if (this.partner == null) {
                this.partner = new SocketPartner();
            }
            this.partner.setPartner(partnerIP, partnerPort);
            this.partner.setOverwrite(overwrite);
        }

        public void setPartner(IPEndPoint _partner, bool overwrite) {
            if (this.isCopy) {
                Console.WriteLine("Can not change the partner of a copy socket!");
                return;
            }
            if (this.partner == null) {
                this.partner = new SocketPartner(_partner, overwrite);
            } else {
                this.partner.setPartner(_partner);
                this.partner.setOverwrite(overwrite);
            }
        }

        public void setPartner(SocketPartner _partner, bool overwrite) {
            if (this.isCopy) {
                Console.WriteLine("Can not change the partner of a copy-socket!");
                return;
            }
            this.partner = _partner;
            this.partner.setOverwrite(overwrite);
        }

        public SocketPartner getPartner() {
            return this.partner;
        }

        public bool isInitialized() {
            return this.initialized;
        }

        public SocketPartner getMyself() {
            return this.myself;
        }

        public void accept(ref Socket acceptSocket, System.Net.Sockets.Socket acceptedSocket, bool verbose = false) {
            if (acceptSocket != null) {
                acceptSocket.cleanup();
                acceptSocket = null;
            }
            acceptSocket = new Socket(null);
            Console.WriteLine("Socket Initialization: accepting tcp connection...");
            acceptSocket.socket = acceptedSocket;
            acceptSocket.partner = new SocketPartner((IPEndPoint)acceptSocket.socket.RemoteEndPoint);

            if (verbose) {
                Console.WriteLine("Found connection...");
            }
            acceptSocket.initMyself(false);
            Console.WriteLine("Socket Initialization: accepted tcp connection from " + acceptSocket.partner.getPartnerString());
            // printSocketDetails(acceptSocket.socket);

            _setSocketBufferSizes(acceptSocket.socket);
            acceptSocket.initialized = true;
        }

        public bool sendBytes(byte[] buffer, ulong bufferLength, ref int errorCode, SerializationHeader header, int retries = 0, bool verbose = false) {
            return this._sendBytes(buffer, bufferLength, ref errorCode, header, retries, verbose);
        }

        public bool sendBytes(utils.Buffer buffer, ref int errorCode, SerializationHeader header, int retries = 0, bool verbose = false) {
            return this._sendBytes(buffer.getBuffer(), buffer.getBufferContentSize(), ref errorCode, header, retries, verbose);
        }

        public bool receiveBytes(ref byte[] buffer, ref ulong bufferLength, ulong expectedLength, ref int errorCode, SerializationHeader expectedHeader, int retries = 0, bool verbose = false) {
            Utils.prepareBuffer(ref buffer, ref bufferLength, expectedLength);
            return this._receiveBytes(buffer, expectedLength, ref errorCode, expectedHeader, retries, verbose);
        }

        public bool receiveBytes(utils.Buffer buffer, ref int errorCode, SerializationHeader expectedHeader, int retries = 0, bool verbose = false) {
            return this._receiveBytes(buffer.getBuffer(), buffer.getBufferContentSize(), ref errorCode, expectedHeader, retries, verbose);
        }

        private static void _setSocketBufferSizes(System.Net.Sockets.Socket socket) {
            socket.SetSocketOption(System.Net.Sockets.SocketOptionLevel.Socket, System.Net.Sockets.SocketOptionName.SendBuffer, Utils.SOCKET_BUFFER_SEND_SIZE);
            socket.SetSocketOption(System.Net.Sockets.SocketOptionLevel.Socket, System.Net.Sockets.SocketOptionName.ReceiveBuffer, Utils.SOCKET_BUFFER_RECV_SIZE);
            Console.WriteLine("Socket send buffer size: " + socket.GetSocketOption(System.Net.Sockets.SocketOptionLevel.Socket, System.Net.Sockets.SocketOptionName.SendBuffer));
            Console.WriteLine("Socket recv buffer size: " + socket.GetSocketOption(System.Net.Sockets.SocketOptionLevel.Socket, System.Net.Sockets.SocketOptionName.ReceiveBuffer));
        }

        private static void _setSocketTimeouts(System.Net.Sockets.Socket socket, int sendTimeout = -1, int recvTimeout = -1) {
            if (sendTimeout > 0) {
                socket.SetSocketOption(System.Net.Sockets.SocketOptionLevel.Socket, System.Net.Sockets.SocketOptionName.SendTimeout, sendTimeout);
            }
            if (recvTimeout > 0) {
                socket.SetSocketOption(System.Net.Sockets.SocketOptionLevel.Socket, System.Net.Sockets.SocketOptionName.ReceiveTimeout, recvTimeout);
            }
        }

        private Socket() {
            this.protocol = SocketType.UDP;
            this.socket = null;
            this.partner = null;
            this.myself = null;
            this.sendTimeout = -1;
            this.recvTimeout = -1;
            this.isCopy = false;
            this.initialized = false;
            this.recvAddress = new IPEndPoint(IPAddress.Any, 0);
            this.sendBuffer = new utils.Buffer(Utils.CLIENT_MAX_MESSAGE_BYTES + 4);
            this.recvBuffer = new utils.Buffer(Utils.CLIENT_MAX_MESSAGE_BYTES + 4);
        }

        private Socket(SocketPartner partner) : this() {
            this.protocol = SocketType.TCP;
            if (partner != null) {
                this.partner = new SocketPartner(partner.getIP(), partner.getPort(), partner.getOverwrite());
            }
        }

        private void createSocket() {
            if (this.socket != null) {
                this.close();
            }

            switch (this.protocol) {
                case SocketType.UDP:
                case SocketType.UDP_HEADER: {
                    this.socket = new System.Net.Sockets.Socket(System.Net.Sockets.AddressFamily.InterNetwork, System.Net.Sockets.SocketType.Dgram, System.Net.Sockets.ProtocolType.Udp);
                    break;
                }
                case SocketType.TCP: {
                    this.socket = new System.Net.Sockets.Socket(System.Net.Sockets.AddressFamily.InterNetwork, System.Net.Sockets.SocketType.Stream, System.Net.Sockets.ProtocolType.Tcp);
                    break;
                }
                default: {
                    throw new Exception("Unknown protocol: " + this.protocol);
                }
            }

            _setSocketBufferSizes(this.socket);
        }

        private bool initMyself(bool withBind = true) {
            if (withBind) {
                // bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
                // bind() passes file descriptor, the address structure, and the length of the address structure
                // This bind() call will bind the socket to the current IP address on port 'portNumber'
                Console.WriteLine("Socket Initialization: binding socket to local " + this.myself.getPartnerString() + "...");
                try {
                    this.socket.Bind(this.myself.getPartner());
                } catch (Exception e) {
                    Console.WriteLine("ERROR on bind... " + e.ToString());
                    throw e;
                    // return false;
                }
            }
            if (this.myself == null || this.myself.getPort() == 0) {
                // this.socket.RemoteEndPoint;
                if (this.myself == null) {
                    this.myself = new SocketPartner((IPEndPoint)this.socket.LocalEndPoint);
                } else {
                    this.myself.setPartner((IPEndPoint)this.socket.LocalEndPoint);
                }
            }
            Console.WriteLine("Socket Initialization: bound socket to local " + this.myself.getPartnerString() + "!");
            return true;
        }

        private bool performSend(byte[] buffer, ref int localBytesSent, ref int errorCode, SerializationHeader header, int sendSize, int sentBytes, byte sendIteration, bool verbose = false) {
            bool withHeader = (header != null || this.protocol == SocketType.UDP_HEADER);
            int localSendOffset;
            if (withHeader) {
                if (header != null) {
                    this.sendBuffer.setByte(header.getSerializationIteration(), 0);
                } else {
                    this.sendBuffer.setByte(0, 0);
                }
                this.sendBuffer.setByte(sendIteration, 1);
                this.sendBuffer.setShort((short)sendSize, 2);
                this.sendBuffer.setData(buffer, (ulong)sendSize, 4, (ulong)sentBytes);
                localSendOffset = 0;
            } else {
                this.sendBuffer.setReferenceToData(buffer, (ulong)sendSize);
                localSendOffset = sentBytes;
            }
            if (verbose) {
                Console.WriteLine("Pre send: already sentBytes = " + sentBytes + "; sending " + sendSize + "B to " + this.socket);
            }

            switch (this.protocol) {
                case SocketType.UDP:
                case SocketType.UDP_HEADER: {
                    IPEndPoint toAddress = this.partner.getPartner();
                    Debug.Assert(toAddress != null);
                    if (toAddress == null) {
                        Console.WriteLine("UDP send partner is null!");
                        errorCode = 0;
                        return false;
                    }
                    localBytesSent = this.socket.SendTo(this.sendBuffer.getBuffer(), localSendOffset, (int) this.sendBuffer.getBufferContentSize(), 0, toAddress);
                    break;
                }
                case SocketType.TCP: {
                    localBytesSent = this.socket.Send(this.sendBuffer.getBuffer(), localSendOffset, (int) this.sendBuffer.getBufferContentSize(), 0);
                    break;
                }
                default: {
                    throw new Exception("Unknown SocketType " + this.protocol);
                }
            }
            if (verbose) {
                Console.WriteLine("Post send: managed to send localBytesSent = " + localBytesSent);
                /*
                for (int i = 0; i < localBytesSent; i++) {
                    Console.Write((int) buffer[i + sentBytes] + ", ");
                }
                Console.WriteLine();
                //*/
            }

            if (withHeader && localBytesSent >= 4) {
                localBytesSent -= 4;
            }
            return true;
        }

        private bool interpretSendResult(byte[] buffer, ref int localBytesSent, ref int errorCode, ref int sentBytes, ref int retries, ref byte sendIteration, SerializationHeader header, int sendSize, bool verbose = false) {
            try {
                if (!this.performSend(buffer, ref localBytesSent, ref errorCode, header, sendSize, sentBytes, sendIteration, verbose)) {
                    return false;
                }
            } catch (System.Net.Sockets.SocketException se) {
                errorCode = se.ErrorCode;
                if (errorCode == 10054) {
                    // WSAECONNRESET
                    Console.WriteLine("Connection closed on send... " + se.ToString());
                    errorCode = 0;
                    return false;
                } else if (errorCode == 10058) {
                    errorCode = 0;
                    return false;
                } else if (errorCode == 10060) {
                    errorCode = 0;
                    localBytesSent = 0;
                } else {
                    Console.WriteLine("ERROR on send... " + se.ToString());
                    return false;
                }
            } catch (ObjectDisposedException ode) {
                Console.WriteLine("Socket closed... " + ode.ToString());
                errorCode = 0;
                return false;
            } catch (Exception e) {
                Console.WriteLine("ERROR on send... " + e.ToString());
                errorCode = 1;
                return false;
            }

            if (localBytesSent == 0) {
                if (verbose) {
                    Console.WriteLine("Sent 0 bytes???");
                }
                if (retries == 0) {
                    errorCode = 0;
                    return false;
                }
                if (retries > 0) {
                    retries--;
                }
            } else {
                sendIteration++;
            }

            return true;
        }

        private bool _sendBytes(byte[] buffer, ulong bufferLength, ref int errorCode, SerializationHeader header, int retries = 0, bool verbose = false) {
            if (!this.isInitialized()) {
                Console.WriteLine("Can not send with an uninitialized socket!");
                return false;
            }
            if (verbose) {
                Console.WriteLine("Entering _sendBytes function with buffer length = " + bufferLength + " and sendPartner " + ((this.partner == null) ? "any" : this.partner.getStringAddress()) + "!");
            }
            int sentBytes = 0, localBytesSent = 0;
            ulong maxSendPerRound = Utils.CLIENT_MAX_MESSAGE_BYTES;
            byte sendIteration = 0;
            while ((ulong)sentBytes < bufferLength) {
                int sendSize = (int)Math.Min(maxSendPerRound, bufferLength - (ulong)sentBytes);
                if (!this.interpretSendResult(buffer, ref localBytesSent, ref errorCode, ref sentBytes, ref retries, ref sendIteration, header, sendSize, verbose)) {
                    return false;
                }
                sentBytes += localBytesSent;
            }
            if (verbose) {
                Console.WriteLine("Exiting _sendBytes function having sent = " + bufferLength + "B to " + ((this.partner == null) ? "any" : this.partner.getStringAddress()) + "!");
            }
            return true;
        }

        private bool checkCorrectReceivePartner(ref bool overwritePartner, bool recvFirstMessage) {
            if (this.protocol == SocketType.TCP) {
                return true;
            }

            Debug.Assert(overwritePartner || this.partner != null);
            if (overwritePartner) {
                Debug.Assert(!recvFirstMessage);
                // if overwritePartner is desired, choose first partner and do not overwrite until recv is finished!
                if (this.partner == null) {
                    this.partner = new SocketPartner((IPEndPoint)this.recvAddress);
                } else {
                    this.partner.setPartner((IPEndPoint)this.recvAddress);
                }
                overwritePartner = false;
            } else if (!Utils.comparePartners((IPEndPoint)this.recvAddress, this.partner.getPartner())) {
                // do not consider received data from other partners than the saved partner
                return false;
            }
            return true;
        }

        private bool performReceive(byte[] buffer, ref int localReceivedBytes, ref bool overwritePartner, ref bool recvFromCorrectPartner,
                                    SerializationHeader expectedHeader, int receiveSize, int receivedBytes, bool recvFirstMessage, bool verbose = false) {
            recvFromCorrectPartner = true;  // needed because there are return paths before recomputing the value :)
            if (verbose) {
                Console.WriteLine("Pre receive... already recvBytes = " + receivedBytes);
            }

            bool withHeader = (expectedHeader != null || this.protocol == SocketType.UDP_HEADER);
            int dataStart = (withHeader) ? 4 : 0;
            if (withHeader && !recvFirstMessage && !this.recvBuffer.empty()) {
                localReceivedBytes = (int)this.recvBuffer.getBufferContentSize();
            } else {
                Debug.Assert(this.recvBuffer.empty());
                Debug.Assert(this.recvBuffer.getBuffer() != null);
                switch (this.protocol) {
                    case SocketType.UDP:
                    case SocketType.UDP_HEADER: {
                        localReceivedBytes = this.socket.ReceiveFrom(this.recvBuffer.getBuffer(), 0, dataStart + receiveSize, 0, ref this.recvAddress);
                        break;
                    }
                    case SocketType.TCP: {
                        localReceivedBytes = this.socket.Receive(this.recvBuffer.getBuffer(), 0, dataStart + receiveSize, 0);
                        break;
                    }
                    default: {
                        throw new Exception("Unknown SocketType " + this.protocol);
                    }
                }
            }

            if (withHeader && localReceivedBytes >= 0) {
                if (localReceivedBytes < 4) {
                    Console.WriteLine("Wrong protocol for this socket!!!");
                    localReceivedBytes = -1;
                    return false;
                }
                this.recvBuffer.setBufferContentSize((ulong)localReceivedBytes);
                localReceivedBytes -= 4;
                if (verbose) {
                    Console.WriteLine("Should have received " + ((ushort)this.recvBuffer.getShort(2)) + ", received " + localReceivedBytes);
                }
            }
            if (verbose) {
                Console.WriteLine("Post receive... localReceivedBytes = " + localReceivedBytes + "; overwritePartner = " + overwritePartner);
            }

            if (localReceivedBytes < 0) {
                return true;
            }

            recvFromCorrectPartner = this.checkCorrectReceivePartner(ref overwritePartner, recvFirstMessage);
            if (!recvFromCorrectPartner) {
                this.recvBuffer.setBufferContentSize(0);
                return true;
            }

            if (withHeader) {
                // check serialization iteration
                if (expectedHeader != null && this.recvBuffer.getByte(0) != expectedHeader.getSerializationIteration()) {
                    // received different serialization state... check if the partner is the same
                    // interrupt receive! and don't reset the recvBuffer to keep data for next recv!
                    localReceivedBytes = -2;
                    return true;
                }

                // check send iteration
                byte sendIteration = this.recvBuffer.getByte(1);
                if (recvFirstMessage && sendIteration == 0) {
                    // Received start of another message... interrupt receive!
                    // And don't reset the recvBuffer to keep data for next recv!
                    localReceivedBytes = -2;
                    return true;
                }
                if (!recvFirstMessage && sendIteration > 0) {
                    // wrong data, ignore, pretend like nothing was received (like recv from wrong partner)
                    recvFromCorrectPartner = false;
                }
            }
            if (recvFromCorrectPartner) {
                if (verbose) {
                    Console.Write("Received data: ");
                    byte[] receivedBufferData = this.recvBuffer.getBuffer();
                    for (int i = 0; i < Math.Min(localReceivedBytes, 50); i++) {
                        Console.Write((int)receivedBufferData[dataStart + i] + " ");
                    }
                    Console.WriteLine();
                }
                utils.Utils.memcpy(buffer, (ulong)receivedBytes, this.recvBuffer.getBuffer(), (ulong)dataStart, (ulong)localReceivedBytes);
            }
            this.recvBuffer.setBufferContentSize(0);

            if (verbose) {
                Console.WriteLine("Post receive... managed to receive localReceivedBytes = " + localReceivedBytes);
                /*
                for (int i = 0; i < localReceivedBytes; i++) {
                    Console.Write((int) buffer[i + receivedBytes] + ", ");
                }
                Console.WriteLine();
                //*/
            }
            return true;
        }

        private bool interpretReceiveResult(byte[] buffer, ref int localReceivedBytes, ref int errorCode, ref bool overwritePartner,
                                            ref bool recvFromCorrectPartner, ref bool recvFirstMessage, SerializationHeader expectedHeader,
                                            int receiveSize, int receivedBytes, bool verbose = false) {
            try {
                if (!this.performReceive(buffer, ref localReceivedBytes, ref overwritePartner, ref recvFromCorrectPartner, expectedHeader, receiveSize, receivedBytes, recvFirstMessage, verbose)) {
                    return false;
                }
            } catch (System.Net.Sockets.SocketException se) {
                errorCode = se.ErrorCode;
                if (errorCode == 10060) {
                    // Timeout!
                    localReceivedBytes = -1;
                    errorCode = 0;
                } else if (errorCode == 10035) {
                    // Timeout on non-blocking socket
                    /*
                    Console.WriteLine("Found a non-blocking socket which received timeout: " +
                        this.protocol.ToString() + " - " + this.myself.getStringAddress() + " . " + this.partner.getStringAddress());
                    //*/
                    localReceivedBytes = -1;
                    errorCode = 0;
                } else if (errorCode == 10054) {
                    errorCode = 0;
                    return false;
                } else if (errorCode == 10058) {
                    errorCode = 0;
                    return false;
                } else {
                    Console.WriteLine("ERROR in receive... " + se.ToString());
                    return false;
                }
            } catch (ObjectDisposedException ode) {
                Console.WriteLine("Socket closed " + ode.ToString());
                errorCode = 0;
                return false;
            } catch (Exception e) {
                Console.WriteLine("ERROR in receive... " + e.ToString());
                errorCode = -3;
                return false;
            }

            if (localReceivedBytes > 0) {
                if (recvFromCorrectPartner) {
                    if (verbose) {
                        Console.WriteLine("Received first data!");
                    }
                    recvFirstMessage = true;
                } else {
                    // simulate receiving nothing!
                    localReceivedBytes = -1;
                    errorCode = 0;
                }
            } else if (localReceivedBytes == 0) {
                // socket closed
                errorCode = 0;
                return false;
            } else {

                if (localReceivedBytes != -1 && localReceivedBytes != -2) {
                    Console.WriteLine("The next assertion will fail: localReceivedBytes = " + localReceivedBytes);
                }
                Debug.Assert(localReceivedBytes == -1 || localReceivedBytes == -2);
                if (localReceivedBytes == -2) {
                    errorCode = -2;
                    return false;
                }
                // only set the errorCode here, because when there is an error, localReceivedBytes is set to - 1!
                Debug.Assert(localReceivedBytes == -1);
                if (verbose) {
                    Console.WriteLine("ReceiveBytes:" + errorCode);
                }
            }

            return true;
        }

        private void setRetries(ref int retries, ref bool retry, int maxRetries, bool recvFromCorrectPartner, int localReceivedBytes, bool verbose = false) {
            // retry receiving if received data from someone else or (if received nothing and there are retries left)
            retry = (!recvFromCorrectPartner) || ((localReceivedBytes == -1) && (retries != 0));
            if (verbose) {
                Console.WriteLine("Retry = " + retry + "; retries = " + retries);
            }
            if (retry && retries > 0) {
                if (recvFromCorrectPartner) {
                    retries--;
                } else {
                    retries = maxRetries;
                }
            }
        }

        private bool _receiveBytes(byte[] buffer, ulong expectedLength, ref int errorCode, SerializationHeader expectedHeader, int retries = 0, bool verbose = false) {
            if (!this.isInitialized()) {
                Console.WriteLine("Can not receive with an uninitialized socket!");
                // throw new Exception("Can not receive with an uninitialized socket!");
                return false;
            }
            if (verbose) {
                Console.WriteLine("Entering _receiveBytes function with expected length = " + expectedLength + " and receivePartner = " + ((this.partner == null) ? "any" : this.partner.getStringAddress()) + "!");
            }
            Debug.Assert((ulong)buffer.Length >= expectedLength);
            int localReceivedBytes = 0, receiveSize, receivedBytes = 0, maxRetries = retries;
            ulong maxPossibleReceiveBytes = Utils.CLIENT_MAX_MESSAGE_BYTES;
            errorCode = 0;
            bool recvFirstMessage = false, retry = true, recvFromCorrectPartner = true;
            bool overwritePartner = (this.partner == null) || (!this.partner.isInitialized()) || this.partner.getOverwrite();
            while ((ulong)receivedBytes < expectedLength) {
                receiveSize = (int)Math.Min(expectedLength - (ulong)receivedBytes, maxPossibleReceiveBytes);
                // wait to receive data from socket (and retry when timeout occurs)
                do {
                    if (!this.interpretReceiveResult(buffer, ref localReceivedBytes, ref errorCode, ref overwritePartner, ref recvFromCorrectPartner,
                                                     ref recvFirstMessage, expectedHeader, receiveSize, receivedBytes, verbose)) {
                        return false;
                    }

                    this.setRetries(ref retries, ref retry, maxRetries, recvFromCorrectPartner, localReceivedBytes, verbose);
                } while (!recvFirstMessage && retry);
                if (verbose && localReceivedBytes >= 0) {
                    Console.WriteLine("Total received bytes so far: " + (receivedBytes + localReceivedBytes));
                }

                Debug.Assert(localReceivedBytes == -1 || localReceivedBytes > 0);
                if (localReceivedBytes < 0) {
                    errorCode = -1;
                    return false;
                }
                receivedBytes += localReceivedBytes;
            }
            Debug.Assert((ulong)receivedBytes == expectedLength);
            if (verbose) {
                Console.WriteLine("Exiting _receiveBytes function having received = " + expectedLength + "B to " + ((this.partner == null) ? "any" : this.partner.getStringAddress()) + "!");
            }
            return true;
        }

        System.Net.Sockets.Socket socket;
        SocketType protocol;
        SocketPartner partner, myself;
        bool isCopy, initialized;
        int recvTimeout, sendTimeout;
        utils.Buffer sendBuffer, recvBuffer;
        EndPoint recvAddress;
    };
}
