using System;
using System.Net;

namespace Comm.utils {
    public class SerializationHeader {
        public SerializationHeader(bool create = true) {
            this.created = create;
            if (this.created) {
                this.localBuffer = new byte[4];
            }
        }

        public SerializationHeader(ref byte[] buffer) {
            this.localBuffer = buffer;
        }

        public SerializationHeader(Buffer buffer) {
            this.passedBuffer = buffer;
        }

        public SerializationHeader(int header) {
            this.setInt(header);
        }

        public SerializationHeader(byte serializationIteration, byte sendIteration, ushort sendSize) {
            this.setData(serializationIteration, sendIteration, sendSize);
        }

        ~SerializationHeader() {
            if (this.created) {
                this.localBuffer = null;
            }
        }

        public void reset() {
            this.setInt(0);
        }

        public void setInt(int header, bool local = true) {
            this.setSendSize((ushort)(header & 65535), local);  // mod 2^16
            header >>= 16;  // div 2^16
            this.setSendIteration((byte)(header & 255));  // mod 2^8
            this.setSerializationIteration((byte)(header >> 8));  // div 2^8
        }

        public void setData(byte _serializationIteration, byte _sendIteration, ushort _sendSize, bool local = true) {
            this.setSerializationIteration(_serializationIteration);
            this.setSendIteration(_sendIteration);
            this.setSendSize(_sendSize, local);
        }

        public void setSerializationIteration(byte _serializationIteration) {
            if (this.passedBuffer != null) {
                this.localBuffer[0] = _serializationIteration;
            } else {
                this.passedBuffer.setByte(_serializationIteration, 0);
            }
        }

        public void setSendIteration(byte _sendIteration) {
            if (this.passedBuffer != null) {
                this.localBuffer[1] = _sendIteration;
            } else {
                this.passedBuffer.setByte(_sendIteration, 1);
            }
        }

        public void setSendSize(ushort _sendSize, bool setLocal = true) {
            short sendSize = (short)((setLocal) ? _sendSize : IPAddress.NetworkToHostOrder(_sendSize));
            if (this.passedBuffer != null) {
                utils.Utils.memcpy(this.localBuffer, 2, BitConverter.GetBytes(sendSize), 0, 2);
            } else {
                this.passedBuffer.setShort(sendSize, 2);
            }
        }

        public void setBuffer(byte[] buffer, bool setLocal, int start = 0) {
            buffer[start] = this.getSerializationIteration();
            buffer[start + 1] = this.getSendIteration();
            ushort x = this.getSendSize(setLocal);
            utils.Utils.memcpy(buffer, (ulong)start + 2, BitConverter.GetBytes(x), 0, NetworkData.shortSize);
        }

        public void setBuffer(utils.Buffer buffer, bool setLocal, int start = 0) {
            buffer.setInt(this.getInt(setLocal), start);
        }

        public byte getSerializationIteration() {
            if (this.passedBuffer != null) {
                return this.localBuffer[0];
            } else {
                return this.passedBuffer.getByte(0);
            }
        }

        public byte getSendIteration() {
            if (this.passedBuffer != null) {
                return this.localBuffer[1];
            } else {
                return this.passedBuffer.getByte(1);
            }
        }

        public ushort getSendSize(bool getLocal = true) {
            ushort sendSize;
            if (this.passedBuffer != null) {
                sendSize = (ushort)BitConverter.ToInt16(this.localBuffer, 2);
            } else {
                sendSize = (ushort)this.passedBuffer.getShort(2);
            }
            if (getLocal) {
                return sendSize;
            } else {
                return (ushort)IPAddress.HostToNetworkOrder((short)sendSize);
            }
        }

        public int getInt(bool getLocal = true) {
            return (this.getSerializationIteration() << 24) + (this.getSendIteration() << 16) + this.getSendSize(getLocal);
        }

        private byte[] localBuffer;
        private bool created;
        private utils.Buffer passedBuffer;
    }
}
