using Comm.data;
using System;

namespace Comm {
    public class CoordinateGetter {
        public CoordinateGetter() {
            this.x = 0.5;
            this.y = 0.5;
            this.touch = true;
            this.buttonFwd = false;
            this.buttonDwn = false;
            this.startTime = DateTime.Now;
        }

        ~CoordinateGetter() { }

        public void resetDrawingArea(bool withTimeReset = true) {
            if (withTimeReset) {
                this.startTime = DateTime.Now;
            }
        }

        public void setData(float x, float y, bool touch) {
            this.x = x;
            this.y = y;
            this.touch = touch;
        }

        public bool getData(CoordinateData data) {
            data.set("touch", this.touch);
            data.set("buttonFwd", this.buttonFwd);
            data.set("buttonDwn", this.buttonDwn);
            data.set("x", x);
            data.set("y", y);
            data.set("time", (long)(DateTime.Now - this.startTime).TotalMilliseconds);
            return true;
        }

        double x, y;
        bool buttonFwd, buttonDwn, touch;
        DateTime startTime;
    };
}
