using System;
using System.Diagnostics;

namespace DesignAIRobotics.Comm.data {
    class BytesData : CommunicationData {
        const int headerSize = sizeof(int);

        public BytesData(int size) {
            this.data = new utils.Buffer((ulong) size);
            this.expectedDataSize = 0;

        }

        ~BytesData() {
            this.data.reset();
        }

        public override MessageType getMessageType() {
            return MessageType.BYTES;
        }

        public override bool serialize(utils.Buffer buffer, int start, bool forceCopy, bool verbose) {
            switch (this.serializeState) {
                case 0: {
                    // header
                    buffer.setBufferContentSize((ulong) start + headerSize);
                    buffer.setInt((int) this.data.getBufferContentSize(), start);
                    if (verbose) {
                        byte[] dataBuffer = buffer.getBuffer();
                        Console.WriteLine("buffer int content: " + (int) dataBuffer[0] + " " + (int) dataBuffer[1] + " " + (int) dataBuffer[2] + " " + (int) dataBuffer[3]);
                    }
                    this.serializeState = 1;
                    return false;
                }
                case 1: {
                    if (forceCopy) {
                        buffer.setData(this.getBuffer(), this.getBufferSize(), (ulong) start);
                    } else {
                        if (start != 0) {
                            throw new System.Exception("Can not set a reference to data not starting at the first position!");
                        }
                        buffer.setReferenceToData(this.getBuffer(), this.getBufferSize());
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
                    return (ulong) this.expectedDataSize;
                }
                default: {
                    throw new System.Exception("Impossible deserialize state... " + this.deserializeState);
                }
            }
        }

        public override bool deserialize(utils.Buffer buffer, int start, bool forceCopy, bool verbose) {
            switch (this.deserializeState) {
                case 0: {
                    this.expectedDataSize = buffer.getInt(start);
                    this.deserializeState = 1;
                    return false;
                }
                case 1: {
                    Debug.Assert((buffer.getBufferContentSize() - (ulong) start) == (ulong) this.expectedDataSize);
                    this.data.setData(buffer.getBuffer(), (ulong) this.expectedDataSize, (ulong) start);
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
            this.data.reset();
            this.expectedDataSize = 0;
        }

        public void setData(byte[] _data, uint dataSize, ulong start) {
            this.data.setData(_data, dataSize, start);
        }

        public void setReferenceToData(byte[] _data, uint dataSize) {
            this.data.setReferenceToData(_data, dataSize);
        }

        public void setByte(byte _data, int position) {
            this.data.setByte(_data, position);
        }

        public void setShort(short _data, int position) {
            this.data.setShort(_data, position);
        }

        public void setInt(int _data, int position) {
            this.data.setInt(_data, position);
        }

        public void setLongLong(long _data, int position) {
            this.data.setLongLong(_data, position);
        }

        public void setFloat(float _data, int position) {
            this.data.setFloat(_data, position);
        }

        public void setDouble(double _data, int position) {
            this.data.setDouble(_data, position);
        }

        public byte getByte(int position) {
            return this.data.getByte(position);
        }

        public short getShort(int position) {
            return this.data.getShort(position);
        }

        public int getInt(int position) {
            return this.data.getInt(position);
        }

        public long getLongLong(int position) {
            return this.data.getLongLong(position);
        }

        public float getFloat(int position) {
            return this.data.getFloat(position);
        }

        public double getDouble(int position) {
            return this.data.getDouble(position);
        }

        public bool empty() {
            return this.data.empty();
        }

        public byte[] getBuffer() {
            return this.data.getBuffer();
        }

        public ulong getBufferSize() {
            ulong size = this.data.getBufferContentSize();
            Debug.Assert(this.deserializeState != 1 || size == (ulong) this.expectedDataSize);
            return size;
        }

        private utils.Buffer data;
        private int expectedDataSize;
    }
}
