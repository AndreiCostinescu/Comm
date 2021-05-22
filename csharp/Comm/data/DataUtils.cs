using System;

namespace Comm.data {
    public static class DataUtils {
        public static CommunicationData createCommunicationData(MessageType messageType) {
            switch (messageType) {
                case MessageType.NOTHING: {
                    return null;
                }
                case MessageType.COORDINATE: {
                    return new CoordinateData();
                }
                case MessageType.IMAGE: {
                    return new ImageData();
                }
                case MessageType.STATUS: {
                    return new StatusData();
                }
                case MessageType.BYTES: {
                    return new BytesData();
                }
                default: {
                    throw new Exception("Unknown message type: " + messageType);
                }
            };
        }
    }
}
