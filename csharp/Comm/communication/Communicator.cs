using Comm.data;
using Comm.socket;
using Comm.utils;
using System;
using System.Diagnostics;

namespace Comm.communication {
    public class Communicator {
        private delegate bool QuitCheck();

        public static bool send(Communication comm, SocketType socketType, CommunicationData data, bool withHeader,
                                bool withMessageType = true, int retries = 0, bool verbose = false) {
            if (data == null) {
                return true;
            }
            if (!comm.transmitData(socketType, data, withHeader, withMessageType, retries, verbose)) {
                if (comm.getErrorCode() > 0) {
                    Console.WriteLine("When sending data... error: " + comm.getErrorString() + "; " + comm.getErrorCode());
                }
                return false;
            }
            return true;
        }

        public static bool send(Communication comm, SocketType socketType, byte[] data, int dataSize, int retries = 0,
                                bool verbose = false) {
            return comm.sendRaw(socketType, data, dataSize, retries, verbose);
        }

        private static bool isReceiveErrorOk(int errorCode, ref MessageType messageType, bool nothingOk) {
            if (errorCode < 0) {
                // nothing received
                messageType = MessageType.NOTHING;
                return nothingOk;
            } else if (errorCode == 0) {
                Console.WriteLine("Socket closed.");
            } else {
                Console.WriteLine("An actual error occurred... " + errorCode);
            }
            return false;
        }

        public static bool syphon(Communication comm, SocketType socketType, ref MessageType messageType,
                                  CommunicationData data, bool withHeader, ref bool quitFlag, int retries = 0,
                                  bool verbose = false, int syphonRetries = 10) {
            switch (messageType) {
                case MessageType.NOTHING: {
                    return true;
                }
                case MessageType.COORDINATE:
                case MessageType.IMAGE:
                case MessageType.STATUS: {
                    if (data == null) {
                        data = DataUtils.createCommunicationData(messageType);
                    }
                    int localRetriesBeforeFail = (syphonRetries < 1) ? 1 : syphonRetries;
                    while (!quitFlag && localRetriesBeforeFail > 0) {
                        if (!comm.recvData(socketType, data, withHeader, true, retries, verbose)) {
                            if (comm.getErrorCode() == -1) {
                                localRetriesBeforeFail--;
                                continue;
                            }
                            // it shouldn't be possible to receive nothing if we already have received the header!!!
                            // set nothingOk to false!
                            return isReceiveErrorOk(comm.getErrorCode(), ref messageType, false);
                        }
                        return true;
                    }
                    return false;
                }
                default: {
                    throw new Exception("Unknown message type: " + messageType);
                }
            }
        }

        public static bool listen(Communication comm, SocketType socketType, ref MessageType messageType,
                                  ref DataCollection _dataCollection, bool withHeader, ref bool quitFlag,
                                  int retries = 0, bool verbose = false) {
            if (!comm.recvMessageType(socketType, ref messageType, withHeader, retries, verbose)) {
                if (isReceiveErrorOk(comm.getErrorCode(), ref messageType, true)) {
                    return true;
                }
                Console.WriteLine("Error when receiving message type... setting \"quit\"");
                messageType = MessageType.STATUS;
                StatusData status = (StatusData)_dataCollection.get(messageType);
                status.setData(Messages.QUIT_MESSAGE);
                return true;
            }

            CommunicationData data = _dataCollection.get(messageType);
            MessageType receivedMessageType = messageType;
            if (!Communicator.syphon(comm, socketType, ref messageType, data, withHeader, ref quitFlag, retries, verbose)) {
                if (!quitFlag) {
                    Console.WriteLine("Error when syphoning data " + MessageTypeConverter.messageTypeToString(receivedMessageType) + "... setting \"quit\"");
                }
                Console.WriteLine("Error when syphoning data " + MessageTypeConverter.messageTypeToString(messageType) + "... setting \"quit\"");
                messageType = MessageType.STATUS;
                StatusData status = (StatusData)_dataCollection.get(messageType);
                status.setData(Messages.QUIT_MESSAGE);
            }
            return true;
        }

        public static bool listenFor(Communication comm, SocketType socketType, CommunicationData data, bool withHeader, ref bool quitFlag,
                                     Reference<bool> timeoutResult = null, int countIgnoreOther = -1, int countIgnore = -1,
                                     int retries = 0, bool verbose = false) {
            Debug.Assert(data != null);
            MessageType messageType = MessageType.NOTHING;
            while (!quitFlag) {
                if (!comm.recvMessageType(socketType, ref messageType, withHeader, retries, verbose)) {
                    if (isReceiveErrorOk(comm.getErrorCode(), ref messageType, true)) {
                        continue;
                    }
                    return false;
                }

                if (messageType != data.getMessageType()) {
                    if (messageType != MessageType.NOTHING) {
                        Console.WriteLine("Wrong messageType... expected " + MessageTypeConverter.messageTypeToString(data.getMessageType()) + "; got " + MessageTypeConverter.messageTypeToString(messageType));
                    }
                    if (!syphon(comm, socketType, ref messageType, null, withHeader, ref quitFlag, retries, verbose)) {
                        return false;
                    }
                    // it can happen that messageType becomes NOTHING because we don't receive any data in Communicator::syphon!
                    if (messageType != MessageType.NOTHING && countIgnoreOther > 0) {
                        countIgnoreOther--;
                    }
                    if (countIgnore > 0) {
                        countIgnore--;
                    }
                    if (countIgnoreOther == 0 || countIgnore == 0) {
                        if (timeoutResult != null) {
                            timeoutResult.Value = true;
                        }
                        return false;
                    }
                } else {
                    break;
                }
            }
            if (quitFlag) {
                return false;
            }
            while (!quitFlag) {
                if (!syphon(comm, socketType, ref messageType, data, withHeader, ref quitFlag, retries, verbose)) {
                    if (comm.getErrorCode() < 0) {
                        continue;
                    }
                    return false;
                }
                break;
            }
            return !quitFlag;
        }
    };
}
