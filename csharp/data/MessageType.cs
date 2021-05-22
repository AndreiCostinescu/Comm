using System;

namespace Comm.data {
    public enum MessageType {
        NOTHING = 0,
        STATUS,
        IMAGE,
        COORDINATE,
        BYTES,
    };

    public static class MessageTypeConverter {
        public static MessageType stringToMessageType(string s) {
            if (s == "nothing") {
                return MessageType.NOTHING;
            } else if (s == "status") {
                return MessageType.STATUS;
            } else if (s == "image") {
                return MessageType.IMAGE;
            } else if (s == "coordinate") {
                return MessageType.COORDINATE;
            } else if (s == "bytes") {
                return MessageType.BYTES;
            }
            throw new Exception("Can not convert " + s + " to MessageType enum");
        }

        public static string messageTypeToString(MessageType messageType) {
            switch (messageType) {
                case MessageType.NOTHING: {
                    return "nothing";
                }
                case MessageType.STATUS: {
                    return "status";
                }
                case MessageType.IMAGE: {
                    return "image";
                }
                case MessageType.COORDINATE: {
                    return "coordinate";
                }
                case MessageType.BYTES: {
                    return "bytes";
                }
                default: {
                    throw new Exception("Undefined MessageType: " + (int) messageType);
                }
            }
        }
    }
}
