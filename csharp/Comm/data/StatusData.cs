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

        public StatusData(string data) : this() {
            this.setData(data);
        }

        public override MessageType getMessageType() {
            if (Enum.IsDefined(typeof(MessageType), (int)this.getDataType())) {
                return (MessageType)this.getDataType();
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
                        Console.WriteLine("buffer int content: " + (int)dataBuffer[0] + " " + (int)dataBuffer[1] + " " + (int)dataBuffer[2] + " " + (int)dataBuffer[3]);
                    }
                    this.serializeState = 1;
                    return false;
                }
                case 1: {
                    if (forceCopy) {
                        buffer.setData(this.data, (ulong)this.dataSize, (ulong)start);
                    } else {
                        if (start != 0) {
                            throw new System.Exception("Can not set a reference to data not starting at the first position!");
                        }
                        buffer.setReferenceToData(this.data, (ulong)this.dataSize);
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
                    return (ulong)this.dataSize;
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
                    Debug.Assert((buffer.getBufferContentSize() - (ulong) start) == (ulong) this.dataSize);
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
            this.dataType = (byte)MessageType.NOTHING;
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
            utils.Utils.memcpy(this.data, 0, _data, 0, (ulong)Math.Min(_data.Length, _dataSize));
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

        private static readonly byte statusDataType = (byte)MessageType.STATUS;

        private byte[] data;
        private int dataSize, dataLength;
        private byte dataType;
    };
}
