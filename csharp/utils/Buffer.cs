using System;
using System.Diagnostics;

namespace Comm.utils {
    public class Buffer {
        public Buffer(ulong bufferSize = 0) {
            this.buffer = null;
            this.bufferSize = 0;
            this.bufferContentSize = 0;
            this.referenceBuffer = null;
            this.useReferenceBuffer = false;
            this.prepareBuffer(bufferSize);
        }

        ~Buffer() {
            this.reset();
        }

        public void reset() {
            this.buffer = null;
            this.bufferSize = 0;
            this.bufferContentSize = 0;
            this.referenceBuffer = null;
            this.useReferenceBuffer = false;
        }

        public Buffer copy() {
            Buffer copy = new Buffer(this.bufferSize);

            Debug.Assert(copy.bufferSize == this.bufferSize);
            Utils.memcpy(copy.buffer, 0, this.buffer, 0, this.bufferSize);

            copy.bufferContentSize = this.bufferContentSize;

            copy.referenceBuffer = this.referenceBuffer;
            copy.useReferenceBuffer = this.useReferenceBuffer;

            return copy;
        }

        public void setData(byte[] data, ulong dataSize, ulong start = 0, ulong dataStart = 0) {
            this.setBufferContentSize(dataSize + start);
            Utils.memcpy(this.buffer, start, data, dataStart, dataSize);
            this.useReferenceBuffer = false;
        }

        public void setReferenceToData(byte[] data, ulong dataSize) {
            this.referenceBuffer = data;
            this.bufferContentSize = dataSize;
            this.useReferenceBuffer = true;
        }

        public void setByte(byte data, int position) {
            this.buffer[position] = data;
        }

        public void setShort(short data, int position) {
            NetworkData.shortToNetworkBytes(this.buffer, position, data);
        }

        public void setInt(int data, int position) {
            NetworkData.intToNetworkBytes(this.buffer, position, data);
        }

        public void setLongLong(long data, int position) {
            NetworkData.longLongToNetworkBytes(this.buffer, position, data);
        }

        public void setFloat(float data, int position) {
            NetworkData.floatToNetworkBytes(this.buffer, position, data);
        }

        public void setDouble(double data, int position) {
            NetworkData.doubleToNetworkBytes(this.buffer, position, data);
        }

        public byte getByte(int position) {
            return buffer[position];
        }

        public short getShort(int position) {
            return NetworkData.networkBytesToShort(this.buffer, position);
        }

        public int getInt(int position) {
            return NetworkData.networkBytesToInt(this.buffer, position);
        }

        public long getLongLong(int position) {
            return NetworkData.networkBytesToLongLong(this.buffer, position);
        }

        public float getFloat(int position) {
            return NetworkData.networkBytesToFloat(this.buffer, position);
        }

        public double getDouble(int position) {
            return NetworkData.networkBytesToDouble(this.buffer, position);
        }

        public bool empty() {
            return (this.bufferContentSize == 0);
        }

        public byte[] getBuffer() {
            if (this.useReferenceBuffer) {
                this.useReferenceBuffer = false;
                return this.referenceBuffer;
            }
            return this.buffer;
        }

        public ulong getBufferSize() {
            return this.bufferSize;
        }

        public ulong getBufferContentSize() {
            return this.bufferContentSize;
        }

        public void setBufferContentSize(ulong _bufferContentSize) {
            this.prepareBuffer(_bufferContentSize);
            this.bufferContentSize = _bufferContentSize;
        }


        private void prepareBuffer(ulong desiredSize) {
            if (this.bufferSize < desiredSize) {
                byte[] oldBuffer = this.buffer;
                ulong oldSize = this.bufferSize;

                // cout << "Initialize new buffer!" << endl;
                this.buffer = new byte[desiredSize];
                this.bufferSize = desiredSize;
                // cout << "Initialized new buffer (" << bufferLength << ")!" << endl;

                Utils.memcpy(this.buffer, 0, oldBuffer, 0, oldSize * sizeof(byte));
                Utils.memset(this.buffer, oldSize, 0, (this.bufferSize - oldSize) * sizeof(byte));
            }
        }

        private void checkBufferContentSize(ulong size, bool modifySize) {
            if (size > this.bufferContentSize) {
                if (modifySize) {
                    this.prepareBuffer(size);
                    this.bufferContentSize = size;
                } else {
                    Console.WriteLine("Size of " + size + " is greater than the current buffer content size " + this.bufferContentSize + "! The next assertion will fail...\n");
                }
            }
            if (size > this.bufferContentSize) {
                Console.WriteLine("Requested size = " + size + " vs. this.bufferContentSize = " + this.bufferContentSize + "\n");
                Debug.Assert(!modifySize);
            }
            Debug.Assert(size <= this.bufferContentSize);
        }

        byte[] buffer, referenceBuffer;
        ulong bufferSize, bufferContentSize;
        bool useReferenceBuffer;
    };
}
