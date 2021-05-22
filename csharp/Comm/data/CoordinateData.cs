using System;

namespace Comm.data {
    public class CoordinateData : CommunicationData {
        public readonly static ulong headerSize = 2 * sizeof(int) + 2 * sizeof(double) + sizeof(long);

        public CoordinateData() {
            this.id = -1;
            this.time = 0;
            this.x = 0;
            this.y = 0;
            this.touch = false;
            this.buttonFwd = false;
            this.buttonDwn = false;
        }

        public override MessageType getMessageType() {
            return MessageType.COORDINATE;
        }

        public override bool serialize(utils.Buffer buffer, int start, bool forceQuit, bool verbose) {
            switch (this.serializeState) {
                case 0: {
                    buffer.setBufferContentSize(headerSize);
                    if (verbose) {
                        Console.WriteLine("Serialize: " + this.id + ", " + this.time + ", " + this.x + ", " + this.y + ", " + this.touch + ", " + this.buttonFwd + ", " + this.buttonDwn);
                    }
                    buffer.setInt(this.id, start);
                    buffer.setLongLong(this.time, start + 4);
                    buffer.setDouble(this.x, start + 12);
                    buffer.setDouble(this.y, start + 20);
                    buffer.setInt(((this.buttonDwn ? 1 : 0) << 2) + ((this.buttonFwd ? 1 : 0) << 1) + (this.touch ? 1 : 0), start + 28);
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
                default: {
                    throw new Exception("Impossible deserialize state... " + this.deserializeState);
                }
            }
        }

        public override bool deserialize(utils.Buffer buffer, int start, bool forceCopy, bool verbose) {
            switch (this.deserializeState) {
                case 0: {
                    this.id = buffer.getInt(start);
                    this.time = buffer.getLongLong(start + 4);
                    this.x = buffer.getDouble(start + 12);
                    this.y = buffer.getDouble(start + 20);
                    int tmp = buffer.getInt(start + 28);
                    this.touch = (tmp & 1) != 0;
                    this.buttonFwd = (tmp & 2) != 0;
                    this.buttonDwn = (tmp & 4) != 0;
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

        public void set(string what, int value) {
            if (what == "id") {
                this.id = value;
            }
        }

        public void set(string what, double value) {
            if (what == "x") {
                this.x = value;
            } else if (what == "y") {
                this.y = value;
            }
        }

        public void set(string what, long value) {
            if (what == "time") {
                this.time = value;
            }
        }

        public void set(string what, bool value) {
            if (what == "touch") {
                this.touch = value;
            } else if (what == "buttonFwd") {
                this.buttonFwd = value;
            } else if (what == "buttonDwn") {
                this.buttonDwn = value;
            }
        }

        public int getID() {
            return this.id;
        }

        public double getX() {
            return this.x;
        }

        public double getY() {
            return this.y;
        }

        public long getTime() {
            return this.time;
        }

        public bool getTouch() {
            return this.touch;
        }

        public bool getButtonFwd() {
            return this.buttonFwd;
        }

        public bool getButtonDwn() {
            return this.buttonDwn;
        }

        private int id;
        private double x, y;
        private long time;
        private bool touch, buttonFwd, buttonDwn;
    };
}
