using Comm.data;
using Comm.socket;
using Comm.utils;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;

namespace Comm.communication {
    public class Communication {
        public Communication() {
            this.sockets = new Dictionary<SocketType, Socket>();
            this.sendBuffer = new utils.Buffer();
            this.recvBuffer = new utils.Buffer();
            this.isCopy = false;
            this.errorCode = 0;
            this.sendHeader = new SerializationHeader();
            this.recvHeader = new SerializationHeader();
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

        public bool transmitData(SocketType socketType, CommunicationData data, bool withHeader, bool withMessageType = true,
                                 int retries = 0, bool verbose = false) {
            if (data == null) {
                return false;
            }
            this.errorCode = 0;

            bool serializeDone = false;
            int dataStart = 0;
            byte serializationState = 0;
            while (!serializeDone) {
                if (withHeader) {
                    this.sendHeader.setData(serializationState, 0, 0);
                }
                if (serializationState == 0 && withMessageType) {
                    this.sendBuffer.setBufferContentSize((ulong)dataStart + 1);
                    this.sendBuffer.setByte((byte)(data.getMessageType()), dataStart);
                } else {
                    serializeDone = data.serialize(this.sendBuffer, dataStart, withHeader, verbose);
                }
                this.errorCode = 0;
                if (!this.send(socketType, withHeader, retries, verbose)) {
                    if (this.errorCode == 0) {
                        if (verbose) {
                            Console.WriteLine("Socket closed: Can not send data serialized bytes...");
                        }
                    } else {
                        Console.WriteLine("Can not send data serialized bytes... error " + this.errorCode);
                    }
                    data.resetSerializeState();
                    return false;
                }
                serializationState++;
            }
            return true;
        }

        public bool sendRaw(SocketType socketType, byte[] data, int dataSize, int retries = 0, bool verbose = false) {
            if (data == null) {
                return false;
            }
            this.sendBuffer.setData(data, (ulong)dataSize);
            this.errorCode = 0;
            if (!this.send(socketType, false, retries, verbose)) {
                if (this.errorCode == 0) {
                    if (verbose) {
                        Console.WriteLine("Socket closed: Can not raw data serialized bytes...");
                    }
                } else {
                    Console.WriteLine("Can not send raw serialized bytes... error " + this.errorCode);
                }
                return false;
            }
            return true;
        }

        public bool recvMessageType(SocketType socketType, ref MessageType messageType, bool withHeader, int retries = 0,
                                    bool verbose = false) {
            bool receiveResult;
            byte[] dataLocalDeserializeBuffer = null;
            ulong expectedSize = 0;
            int dataStart = 0;
            this.errorCode = 0;
            if (withHeader) {
                this.recvHeader.setData(0, 0, 0);
            }
            this.preReceiveMessageType(ref dataLocalDeserializeBuffer, ref expectedSize, dataStart);
            receiveResult = this.doReceive(socketType, ref dataLocalDeserializeBuffer, ref expectedSize, withHeader, retries, verbose);
            return this.postReceiveMessageType(ref messageType, receiveResult, dataStart);
        }

        public bool recvData(SocketType socketType, CommunicationData data, bool withHeader, bool gotMessageType = true,
                             int retries = 0, bool verbose = false) {
            bool receiveResult, deserializationDone = false, receivedSomething = false;
            int deserializeState = (int)(gotMessageType ? 1 : 0), localRetriesThreshold = 0, localRetries = localRetriesThreshold;
            byte[] dataLocalDeserializeBuffer = null;
            ulong expectedSize = 0;
            int dataStart = 0;
            MessageType messageType = data.getMessageType();
            while (!deserializationDone && localRetries >= 0) {
                this.errorCode = 0;
                if (withHeader) {
                    this.recvHeader.setData((byte)deserializeState, 0, 0);
                }
                this.preReceiveData(ref dataLocalDeserializeBuffer, ref expectedSize, dataStart, data, withHeader);
                receiveResult = this.doReceive(socketType, ref dataLocalDeserializeBuffer, ref expectedSize, withHeader, retries,
                                               verbose);
                if (!this.postReceiveData(ref data, ref deserializeState, ref localRetries, ref receivedSomething, ref deserializationDone,
                                           messageType, dataStart, localRetriesThreshold, receiveResult, withHeader, verbose)) {
                    return false;
                }
            }
            if (receivedSomething && !deserializationDone) {
                Console.WriteLine("After loop: Could not recv " + MessageTypeConverter.messageTypeToString(messageType) +
                                  " serialized bytes... error " + this.errorCode + "; deserializeState = " + deserializeState);
                if (data != null) {
                    data.resetDeserializeState();
                }
                return false;
            } else if (!receivedSomething) {
                Debug.Assert(this.errorCode == -1);
                if (verbose) {
                    Console.WriteLine("Did not receive anything although expected " + MessageTypeConverter.messageTypeToString(messageType));
                }
                return false;
            }
            return true;
        }

        public bool receiveData(SocketType socketType, DataCollection data, bool withHeader, int retries = 0, bool verbose = false) {
            bool deserializationDone = false, receiveResult, receivedSomething = false;
            int deserializeState = 0;
            int localRetriesThreshold = 0, localRetries = localRetriesThreshold;  // 10
            ulong expectedSize = 0;
            byte[] dataLocalDeserializeBuffer = null;
            MessageType messageType = 0;
            CommunicationData recvData = null;
            int dataStart = 0;

            while (!deserializationDone && localRetries >= 0) {
                this.errorCode = 0;

                // receive setup
                if (deserializeState == 0) {
                    this.preReceiveMessageType(ref dataLocalDeserializeBuffer, ref expectedSize, dataStart);
                } else {
                    this.preReceiveData(ref dataLocalDeserializeBuffer, ref expectedSize, dataStart, recvData, withHeader);
                    if (verbose) {
                        Console.WriteLine("In Communication::fullReceiveData: dataLocalDeserializeBuffer = " + dataLocalDeserializeBuffer +
                                          "; expectedSize = " + expectedSize + "; deserializeState = " + deserializeState);
                    }
                }

                // do receive
                receiveResult = this.doReceive(socketType, ref dataLocalDeserializeBuffer, ref expectedSize, withHeader, retries, verbose);

                // post receive
                if (deserializeState == 0) {
                    if (!this.postReceiveMessageType(ref messageType, receiveResult, dataStart)) {
                        return false;
                    }

                    if (messageType == MessageType.NOTHING) {
                        deserializationDone = true;
                        break;
                    } else {
                        recvData = data.get(messageType);
                    }
                } else {
                    if (!this.postReceiveData(ref recvData, ref deserializeState, ref localRetries, ref receivedSomething, ref deserializationDone,
                                              messageType, dataStart, localRetriesThreshold, receiveResult, withHeader, verbose)) {
                        return false;
                    }
                }

                deserializeState++;
            }

            if (receivedSomething && !deserializationDone) {
                Console.WriteLine("After loop: Could not recv " + MessageTypeConverter.messageTypeToString(messageType) +
                                  " serialized bytes... error " + this.errorCode + "; deserializeState = " + deserializeState);
                if (recvData != null) {
                    recvData.resetDeserializeState();
                }
                return false;
            } else if (!receivedSomething) {
                Debug.Assert(this.errorCode == -1);
                if (verbose) {
                    Console.WriteLine("Did not receive anything although expected " + MessageTypeConverter.messageTypeToString(messageType));
                }
                return false;
            }
            return true;
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

        public void setSocketTimeouts(SocketType socketType, int _sendTimeout = -1, int _recvTimeout = -1) {
            Socket socket = this.getSocket(socketType);
            if (socket == null) {
                return;
            }
            socket.setSocketTimeouts(_sendTimeout, _recvTimeout);
        }

        public void setSocket(SocketType socketType, Socket socket) {
            if (this.sockets.ContainsKey(socketType) && this.sockets[socketType] != null) {
                this.sockets[socketType].cleanup();
            }
            this.sockets[socketType] = socket;
        }

        public void setPartner(SocketType socketType, IPEndPoint _partner, bool overwrite) {
            Socket socket = this.getSocket(socketType);
            if (socket == null) {
                return;
            }
            socket.setPartner(_partner, overwrite);
        }

        public void setPartner(SocketType socketType, string partnerIP, int partnerPort, bool overwrite) {
            Socket socket = this.getSocket(socketType);
            if (socket == null) {
                return;
            }
            socket.setPartner(partnerIP, partnerPort, overwrite);
        }

        public void setOverwritePartner(SocketType socketType, bool overwrite) {
            Socket socket = this.getSocket(socketType);
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

        public SocketPartner getMyself(SocketType socketType) {
            Socket socket = this.getSocket(socketType);
            if (socket == null || !socket.isInitialized()) {
                System.Console.WriteLine(SocketTypeConverter.socketTypeToString(socketType) + " socket is not initialized! Can't getMyself...");
                return null;
            }
            return socket.getMyself();
        }

        public string getMyAddressString(SocketType socketType) {
            SocketPartner myself = this.getMyself(socketType);
            Debug.Assert(myself != null);
            return myself.getPartnerString();
        }

        public SocketPartner getPartner(SocketType socketType) {
            Socket socket = this.getSocket(socketType);
            if (socket == null || !socket.isInitialized()) {
                System.Console.WriteLine(SocketTypeConverter.socketTypeToString(socketType) + " socket is not initialized! Can't getPartner...");
                return null;
            }
            return socket.getPartner();
        }

        public string getPartnerString(SocketType socketType) {
            SocketPartner partner = this.getPartner(socketType);
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

        protected virtual bool send(SocketType socketType, bool withHeader, int retries, bool verbose) {
            return this.send(socketType, this.sendBuffer.getBuffer(), this.sendBuffer.getBufferContentSize(),
                ((withHeader) ? this.sendHeader : null), retries, verbose);
        }

        protected virtual bool send(SocketType socketType, byte[] buffer, ulong contentSize, SerializationHeader header, int retries, bool verbose) {
            Socket socket = this.getSocket(socketType);
            if (socket == null) {
                return false;
            }
            return socket.sendBytes(buffer, contentSize, ref this.errorCode, header, retries, verbose);
        }

        protected void preReceiveMessageType(ref byte[] dataLocalDeserializeBuffer, ref ulong expectedSize, int dataStart) {
            dataLocalDeserializeBuffer = null;
            expectedSize = (ulong)dataStart + 1;
        }

        protected void preReceiveData(ref byte[] dataLocalDeserializeBuffer, ref ulong expectedSize, int dataStart,
                                      CommunicationData recvData, bool withHeader) {
            dataLocalDeserializeBuffer = recvData.getDeserializeBuffer();
            // don't to the if before, because some CommunicationData depend on calling getDeserializeBuffer!
            if (withHeader) {
                dataLocalDeserializeBuffer = null;
            }
            expectedSize = (ulong)dataStart + recvData.getExpectedDataSize();
        }


        protected bool doReceive(SocketType socketType, ref byte[] dataLocalDeserializeBuffer, ref ulong expectedSize,
                                 bool withHeader, int retries, bool verbose) {
            if (dataLocalDeserializeBuffer != null) {
                Debug.Assert(!withHeader);
                return this.recv(socketType, ref dataLocalDeserializeBuffer, ref expectedSize, expectedSize, null, retries, verbose);
            } else {
                this.recvBuffer.setBufferContentSize((ulong)expectedSize);
                return this.recv(socketType, withHeader, retries, verbose);
            }
        }

        protected bool postReceiveMessageType(ref MessageType messageType, bool receiveResult, int dataStart) {
            if (!receiveResult) {
                if (this.getErrorCode() < 1) {
                    messageType = MessageType.NOTHING;
                } else {
                    return false;
                }
            } else {
                messageType = (MessageType)this.recvBuffer.getByte(dataStart);
            }
            return true;
        }

        protected bool postReceiveData(ref CommunicationData recvData, ref int deserializeState, ref int localRetries, ref bool receivedSomething,
                                       ref bool deserializationDone, MessageType messageType, int dataStart, int localRetriesThreshold,
                                       bool receiveResult, bool withHeader, bool verbose) {
            if (!receiveResult) {
                if (this.getErrorCode() >= 0) {
                    Console.WriteLine("Stop loop: Can not recv data serialized bytes... error " + this.getErrorCode() + "; deserializeState = " + deserializeState);
                    recvData.resetDeserializeState();
                    return false;
                } else if (this.getErrorCode() == -2) {
                    Console.WriteLine("Stop loop: Only part of the data (" + MessageTypeConverter.messageTypeToString(messageType) + ") has been received before new message started...; deserializeState = " + deserializeState);
                    recvData.resetDeserializeState();
                    return false;
                }
            }

            Debug.Assert(receiveResult || this.getErrorCode() == -1);
            // if we received something...
            if (this.getErrorCode() != -1) {
                if (verbose) {
                    Console.WriteLine("Received something! data.getMessageType() " + MessageTypeConverter.messageTypeToString(messageType));
                }
                receivedSomething = true;
                deserializationDone = recvData.deserialize(this.recvBuffer, dataStart, withHeader, verbose);
                deserializeState++;
                localRetries = localRetriesThreshold;
            } else {
                if (verbose) {
                    Console.WriteLine("Received nothing, decrease local retries!");
                }
                localRetries--;
            }
            return true;
        }

        protected virtual bool recv(SocketType socketType, bool withHeader, int retries, bool verbose) {
            Socket socket = this.getSocket(socketType);
            if (socket == null) {
                return false;
            }
            return socket.receiveBytes(this.recvBuffer, ref this.errorCode, ((withHeader) ? this.recvHeader : null), retries, verbose);
        }

        protected virtual bool recv(SocketType socketType, ref byte[] buffer, ref ulong bufferSize, ulong expectedBytes, SerializationHeader expectedHeader, int retries, bool verbose) {
            Socket socket = this.getSocket(socketType);
            if (socket == null) {
                return false;
            }
            return socket.receiveBytes(ref buffer, ref bufferSize, expectedBytes, ref this.errorCode, expectedHeader, retries, verbose);
        }

        protected Dictionary<SocketType, Socket> sockets;
        protected utils.Buffer sendBuffer, recvBuffer;
        protected bool isCopy;
        protected int errorCode;
        protected SerializationHeader sendHeader, recvHeader;

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
