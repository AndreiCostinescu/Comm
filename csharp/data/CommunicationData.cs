namespace DesignAIRobotics.Comm.data {
    public abstract class CommunicationData {
        public CommunicationData() {
            this.serializeState = 0;
            this.deserializeState = 0;
        }

        ~CommunicationData() { }

        public abstract MessageType getMessageType();

        public void resetSerializeState() {
            this.serializeState = 0;
        }

        public abstract bool serialize(utils.Buffer buffer, int start, bool forceCopy, bool verbose);

        public abstract ulong getExpectedDataSize();

        public virtual byte[] getDeserializeBuffer() {
            return null;
        }

        public void resetDeserializeState() {
            this.deserializeState = 0;
        }

        public abstract bool deserialize(utils.Buffer buffer, int start, bool forceCopy, bool verbose);

        protected int serializeState, deserializeState;
    };
}
