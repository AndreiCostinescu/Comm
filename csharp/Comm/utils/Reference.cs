namespace Comm.utils {
    public class Reference<T> {
        private T v;
        public T Value {
            get => this.v;
            set {
                this.v = value;
            }
        }

        public Reference() { }
    }
}
