using System;
using System.Diagnostics;
using System.Text;

namespace Comm.data {
    public class StatusData : CommunicationData {
        public static readonly ulong headerSize = sizeof(int);

        public StatusData() {
            this.data = null;
            this.dataLength = 0;
            this.dataSize = 0;
            this.dataType = 0;
        }

        public StatusData(string command) : this() {
            this.setCommand(command);
        }

        public override MessageType getMessageType() {
            if (Enum.IsDefined(typeof(MessageType), (int) this.getDataType())) {
                return (MessageType) this.getDataType();
            } else {
                return MessageType.NOTHING;
            }
        }

        public override bool serialize(utils.Buffer buffer, int start, bool forceCopy, bool verbose) {
            switch (this.serializeState) {
                case 0: {
                    buffer.setBufferContentSize(headerSize);
                    buffer.setInt(this.dataSize, 0);
                    if (verbose) {
                        byte[] dataBuffer = buffer.getBuffer();
                        Console.WriteLine("buffer int content: " + (int) dataBuffer[0] + " " + (int) dataBuffer[1] + " " + (int) dataBuffer[2] + " " + (int) dataBuffer[3]);
                    }
                    this.serializeState = 1;
                    return false;
                }
                case 1: {
                    if (forceCopy) {
                        buffer.setData(this.data, (ulong) this.dataSize, (ulong) start);
                    } else {
                        if (start != 0) {
                            throw new System.Exception("Can not set a reference to data not starting at the first position!");
                        }
                        buffer.setReferenceToData(this.data, (ulong) this.dataSize);
                    }
                    this.serializeState = 0;
                    return true;
                }
                default: {
                    Console.WriteLine("Impossible serialize state... " + this.serializeState);
                    this.serializeState = 0;
                    return false;
                }
            }
        }

        public override ulong getExpectedDataSize() {
            switch (this.deserializeState) {
                case 0: {
                    return headerSize;
                }
                case 1: {
                    return (ulong) this.dataSize;
                }
                default: {
                    throw new Exception("Impossible deserialize state... " + this.deserializeState);
                }
            }
        }

        public override bool deserialize(utils.Buffer buffer, int start, bool forceCopy, bool verbose) {
            switch (this.deserializeState) {
                case 0: {
                    this.dataSize = buffer.getInt(start);
                    this.deserializeState = 1;
                    return false;
                }
                case 1: {
                    Debug.Assert(buffer.getBufferContentSize() == (ulong) this.dataSize);
                    this.setData(buffer.getBuffer(), this.dataSize);
                    this.deserializeState = 0;
                    return true;
                }
                default: {
                    Console.WriteLine("Impossible deserialize state... " + this.deserializeState);
                    this.resetDeserializeState();
                    return false;
                }
            }
        }

        public void reset() {
            this.dataSize = -1;
            this.dataType = (byte) MessageType.NOTHING;
        }

        public void setCommand(string command) {
            string commandData = "";
            if (command == "_reset") {
                this.reset();
                return;
            } else if (command == "ping") {
                commandData = Messages.PING_MESSAGE;
                this.dataSize = Messages.PING_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            } else if (command == "quit") {
                commandData = Messages.QUIT_MESSAGE;
                this.dataSize = Messages.QUIT_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            } else if (command == "start") {
                commandData = Messages.START_MESSAGE;
                this.dataSize = Messages.START_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            } else if (command == "stop") {
                commandData = Messages.STOP_MESSAGE;
                this.dataSize = Messages.STOP_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            } else if (command == "wait") {
                commandData = Messages.WAIT_MESSAGE;
                this.dataSize = Messages.WAIT_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            } else if (command == "accept") {
                commandData = Messages.ACCEPT_MESSAGE;
                this.dataSize = Messages.ACCEPT_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            } else if (command == "ready") {
                commandData = Messages.READY_MESSAGE;
                this.dataSize = Messages.READY_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            } else if (command == "control") {
                commandData = Messages.CONTROL_MESSAGE;
                this.dataSize = Messages.CONTROL_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            } else if (command == "upload") {
                commandData = Messages.UPLOAD_MESSAGE;
                this.dataSize = Messages.UPLOAD_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            } else if (command == "select") {
                commandData = Messages.SELECT_MESSAGE;
                this.dataSize = Messages.SELECT_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            } else if (command == "reject") {
                commandData = Messages.REJECT_MESSAGE;
                this.dataSize = Messages.REJECT_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            }
            socket.Utils.prepareBuffer(ref this.data, ref this.dataLength, this.dataSize);
            this.setData(commandData, this.dataSize);
        }

        public void setStatus(string status) {
            string statusData = "";
            if (status == "_reset") {
                this.reset();
                return;
            } else if (status == "idle") {
                statusData = Messages.IDLE_MESSAGE;
                this.dataSize = Messages.IDLE_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            } else if (status == "active") {
                statusData = Messages.ACTIVE_MESSAGE;
                this.dataSize = Messages.ACTIVE_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            } else if (status == "done") {
                statusData = Messages.DONE_MESSAGE;
                this.dataSize = Messages.DONE_MESSAGE_LENGTH;
                this.dataType = (byte) MessageType.STATUS;
            }
            socket.Utils.prepareBuffer(ref this.data, ref this.dataLength, this.dataSize);
            this.setData(statusData, this.dataSize);
        }

        public void setData(byte[] _data) {
            this.setData(_data, _data.Length + 1);
        }

        public void setData(byte[] _data, int _dataSize) {
            if (_data == null) {
                this.dataSize = -1;
                return;
            }
            socket.Utils.prepareBuffer(ref this.data, ref this.dataLength, _dataSize);
            utils.Utils.memcpy(this.data, 0, _data, 0, (ulong) Math.Min(_data.Length, _dataSize));
            this.dataSize = _dataSize;
            this.dataType = statusDataType;
        }

        public void setData(string _data) {
            this.setData(_data, _data.Length + 1);
        }

        public void setData(string _data, int _dataSize) {
            this.setData(Encoding.ASCII.GetBytes(_data), _dataSize);
        }

        public string getData() {
            if (this.dataSize < 0) {
                return null;
            }
            return Encoding.ASCII.GetString(this.data);
        }

        public byte[] getDataBytes() {
            if (this.dataSize < 0) {
                return null;
            }
            return this.data;
        }

        public int getDataSize() {
            return this.dataSize;
        }

        public byte getDataType() {
            return this.dataType;
        }

        private static readonly byte statusDataType = (byte) MessageType.STATUS;

        private byte[] data;
        private int dataSize, dataLength;
        private byte dataType;
    };
}
