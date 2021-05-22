using System.Collections.Generic;

namespace Comm.data {
    public class DataCollection {

        public DataCollection() {
            this.data = new Dictionary<string, CommunicationData>();
        }

        ~DataCollection() {
            this.data.Clear();
        }

        public void set(MessageType messageType, CommunicationData commData) {
            string stringMessageType = MessageTypeConverter.messageTypeToString(messageType);
            this.data[stringMessageType] = commData;
        }

        public CommunicationData get(MessageType messageType) {
            CommunicationData commData;
            if (this.data.TryGetValue(MessageTypeConverter.messageTypeToString(messageType), out commData)) {
                return commData;
            }

            commData = DataUtils.createCommunicationData(messageType);
            this.data.Add(MessageTypeConverter.messageTypeToString(messageType), commData);
            return commData;
        }

        private Dictionary<string, CommunicationData> data;
    };
}
