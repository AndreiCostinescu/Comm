using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using DesignAIRobotics.Comm.data;
using DesignAIRobotics.Comm.socket;

namespace DesignAIRobotics.Comm.communication {
    public class Communication {
        public Communication() {
            this.sockets = new Dictionary<SocketType, Socket>();
            this.sendBuffer = new socket.Buffer();
            this.recvBuffer = new socket.Buffer();
            this.isCopy = false;
            this.errorCode = 0;
            this.dataCollection = new DataCollection();
        }

        ~Communication() {
            this.cleanup();
        }

        public virtual void cleanup() {
            this._cleanup();
            this.isCopy = false;
        }

        public virtual Communication copy() {
            Communication copy = new Communication();
            this.copyData(copy);
            return copy;
        }

        public void createSocket(SocketType socketType, SocketPartner partner = null, int myPort = 0, int sendTimeout = -1, int recvTimeout = -1) {
            Socket socket = this.getSocket(socketType);
            if (socket != null) {
                socket.cleanup();
                socket.initialize(socketType, partner, myPort, sendTimeout, recvTimeout);
            } else {
                this.setSocket(socketType, new Socket(socketType, partner, myPort, sendTimeout, recvTimeout));
            }
        }

        public void setSocketTimeouts(int _sendTimeout = -1, int _recvTimeout = -1) {
            foreach (KeyValuePair<SocketType, Socket> entry in this.sockets) {
                if (entry.Value != null) {
                    this.setSocketTimeouts(entry.Key, _sendTimeout, _recvTimeout);
                }
            }
        }

        public void setSocketTimeouts(SocketType type, int _sendTimeout = -1, int _recvTimeout = -1) {
            Socket socket = this.getSocket(type);
            if (socket == null) {
                return;
            }
            socket.setSocketTimeouts(_sendTimeout, _recvTimeout);
        }

        public bool sendMessageType(SocketType type, MessageType messageType, int retries = 0, bool verbose = false) {
            this.errorCode = 0;
            this.sendBuffer.setBufferContentSize(1);
            this.sendBuffer.setByte((byte)messageType, 0);
            if (!this.send(type, retries, verbose)) {
                Console.WriteLine("Can not send messageType... " + MessageTypeConverter.messageTypeToString(messageType));
                if (verbose) {
                    Console.WriteLine("Can not send messageType... " + MessageTypeConverter.messageTypeToString(messageType));
                }
                return false;
            }
            return true;
        }

        public bool sendData(SocketType type, CommunicationData data, bool withMessageType, int retries = 0, bool verbose = false) {
            if (data == null) {
                return false;
            }
            this.errorCode = 0;

            if (withMessageType) {
                if (!this.sendMessageType(type, data.getMessageType(), retries, verbose)) {
                    Console.WriteLine("Can not send data message type bytes... error " + this.errorCode);
                    return false;
                }
            }

            bool serializeDone = false;
            while (!serializeDone) {
                serializeDone = data.serialize(this.sendBuffer, verbose);
                this.errorCode = 0;
                if (!this.send(type, retries, verbose)) {
                    Console.WriteLine("Can not send data serialized bytes... error " + this.errorCode);
                    data.resetSerializeState();
                    return false;
                }
            }
            return true;
        }

        public bool recvMessageType(SocketType type, ref MessageType messageType, int retries = 0, bool verbose = false) {
            this.errorCode = 0;
            this.recvBuffer.setBufferContentSize(1);

            bool result = this.recv(type, retries, verbose);

            if (!result) {
                // why < 1 (why is also 0, when the socket closes, ok???)
                if (this.errorCode < 1) {
                    messageType = MessageType.NOTHING;
                    return true;
                }
                return false;
            }
            messageType = (MessageType)this.recvBuffer.getByte(0);
            return true;
        }

        public bool recvData(SocketType type, CommunicationData data, int retries = 0, bool verbose = false) {
            Debug.Assert(data != null);
            this.errorCode = 0;
            int localRetriesThreshold = 0, localRetries = localRetriesThreshold, deserializeState = 0;  // 10
            ulong expectedSize;
            bool deserializeDone = false, receiveResult, receivedSomething = false;
            byte[] dataLocalDeserializeBuffer = null;
            while (!deserializeDone && localRetries >= 0) {
                if (verbose) {
                    Console.WriteLine("Communication::recvData: LocalRetries = " + localRetries + " data->getMessageType() " + MessageTypeConverter.messageTypeToString(data.getMessageType()) + "; deserializeState = " + deserializeState);
                }
                this.errorCode = 0;

                dataLocalDeserializeBuffer = data.getDeserializeBuffer();
                expectedSize = data.getExpectedDataSize();
                if (verbose) {
                    Console.WriteLine("In Communication::recvData: dataLocalDeserializeBuffer = " + dataLocalDeserializeBuffer + "; expectedSize = " + expectedSize + "; deserializeState = " + deserializeState);
                }
                if (dataLocalDeserializeBuffer != null) {
                    receiveResult = this.recv(type, ref dataLocalDeserializeBuffer, ref expectedSize, expectedSize, retries, verbose);
                    if (verbose) {
                        Console.WriteLine("ReceiveResult = " + receiveResult);
                    }
                } else {
                    this.recvBuffer.setBufferContentSize(expectedSize);
                    receiveResult = this.recv(type, retries, verbose);
                }
                if (!receiveResult && this.errorCode != -1) {
                    Console.WriteLine("In loop: Can not recv data serialized bytes... error " + this.errorCode + "; deserializeState = " + deserializeState);
                    data.resetDeserializeState();
                    return false;
                }
                // if we received something...
                if (this.errorCode != -1) {
                    if (verbose) {
                        Console.WriteLine("Received something! data->getMessageType() " + MessageTypeConverter.messageTypeToString(data.getMessageType()));
                    }
                    receivedSomething = true;
                    deserializeDone = data.deserialize(this.recvBuffer, 0, verbose);
                    deserializeState++;
                    localRetries = localRetriesThreshold;
                } else {
                    if (verbose) {
                        Console.WriteLine("Received nothing, decrease local retries!");
                    }
                    localRetries--;
                }
            }
            if (receivedSomething && !deserializeDone) {
                Console.WriteLine("After loop: Could not recv data serialized bytes... error " + this.errorCode + "; deserializeState = " + deserializeState);
                Console.WriteLine("After loop: Could not recv data serialized bytes... error " + this.errorCode + "; deserializeState = " + deserializeState);
                data.resetDeserializeState();
                return false;
            } else if (!receivedSomething) {
                Debug.Assert(this.errorCode == -1);
                Console.WriteLine("Did not receive anything although expected " + MessageTypeConverter.messageTypeToString(data.getMessageType()));
                return false;
            }
            return true;
        }

        public void setSocket(SocketType type, Socket socket) {
            if (this.sockets.ContainsKey(type) && this.sockets[type] != null) {
                this.sockets[type].cleanup();
            }
            this.sockets[type] = socket;
        }

        public void setPartner(SocketType type, IPEndPoint _partner, bool overwrite) {
            Socket socket = this.getSocket(type);
            if (socket == null) {
                return;
            }
            socket.setPartner(_partner, overwrite);
        }

        public void setPartner(SocketType type, string partnerIP, int partnerPort, bool overwrite) {
            Socket socket = this.getSocket(type);
            if (socket == null) {
                return;
            }
            socket.setPartner(partnerIP, partnerPort, overwrite);
        }

        public void setOverwritePartner(SocketType type, bool overwrite) {
            Socket socket = this.getSocket(type);
            if (socket == null) {
                return;
            }
            socket.setOverwritePartner(overwrite);
        }

        public Socket getSocket(SocketType socketType) {
            if (!this.sockets.ContainsKey(socketType)) {
                this.sockets[socketType] = null;
            }
            return this.sockets[socketType];
        }

        public SocketPartner getMyself(SocketType type) {
            Socket socket = this.getSocket(type);
            if (socket == null || !socket.isInitialized()) {
                System.Console.WriteLine(SocketTypeConverter.socketTypeToString(type) + " socket is not initialized! Can't getMyself...");
                return null;
            }
            return socket.getMyself();
        }

        public string getMyAddressString(SocketType type) {
            SocketPartner myself = this.getMyself(type);
            Debug.Assert(myself != null);
            return myself.getPartnerString();
        }

        public SocketPartner getPartner(SocketType type) {
            Socket socket = this.getSocket(type);
            if (socket == null || !socket.isInitialized()) {
                System.Console.WriteLine(SocketTypeConverter.socketTypeToString(type) + " socket is not initialized! Can't getPartner...");
                return null;
            }
            return socket.getPartner();
        }

        public string getPartnerString(SocketType type) {
            SocketPartner partner = this.getPartner(type);
            if (partner == null) {
                return "???";
            }
            return partner.getPartnerString();
        }

        public int getErrorCode() {
            return this.errorCode;
        }

        public string getErrorString() {
            if (this.errorCode == -1) {
                return "timeout";
            } else if (this.errorCode == 0) {
                return "closed";
            } else {
                return "errorCode > 0!";
            }
        }

        protected virtual void copyData(Communication copy) {
            copy.isCopy = true;
            foreach (KeyValuePair<SocketType, Socket> entry in this.sockets) {
                if (entry.Value != null) {
                    copy.sockets[entry.Key] = entry.Value.copy();
                }
            }
        }

        protected virtual void _cleanup() {
            this._cleanupData();
        }

        protected virtual bool send(SocketType type, int retries, bool verbose) {
            return this.send(type, this.sendBuffer.getBuffer(), this.sendBuffer.getBufferContentSize(), retries, verbose);
        }

        protected virtual bool send(SocketType type, byte[] buffer, ulong contentSize, int retries, bool verbose) {
            Socket socket = this.getSocket(type);
            if (socket == null) {
                return false;
            }
            return socket.sendBytes(buffer, contentSize, ref this.errorCode, retries, verbose);
        }

        protected virtual bool recv(SocketType type, int retries, bool verbose) {
            Socket socket = this.getSocket(type);
            if (socket == null) {
                return false;
            }
            return socket.receiveBytes(this.recvBuffer, ref this.errorCode, retries, verbose);
        }

        protected virtual bool recv(SocketType type, ref byte[] buffer, ref ulong bufferSize, ulong expectedBytes, int retries, bool verbose) {
            Socket socket = this.getSocket(type);
            if (socket == null) {
                return false;
            }
            return socket.receiveBytes(ref buffer, ref bufferSize, expectedBytes, ref this.errorCode, retries, verbose);
        }

        protected Dictionary<SocketType, Socket> sockets;
        protected socket.Buffer sendBuffer, recvBuffer;
        protected bool isCopy;
        protected int errorCode;
        protected DataCollection dataCollection;

        private void _cleanupData() {
            foreach (KeyValuePair<SocketType, Socket> entry in this.sockets) {
                if (entry.Value != null) {
                    entry.Value.cleanup();
                }
            }
            this.sockets.Clear();
        }
    };
}
