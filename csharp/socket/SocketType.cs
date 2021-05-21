using System;
using System.Collections.Generic;
using System.Text;

namespace DesignAIRobotics.Comm.socket {
    public enum SocketType {
        UDP,
        UDP_HEADER,
        TCP,
    }

    public static class SocketTypeConverter {
        public static SocketType stringToSocketType(string s) {
            if (s == "UDP") {
                return SocketType.UDP;
            } else if (s == "UDP_HEADER") {
                return SocketType.UDP_HEADER;
            } else if (s == "TCP") {
                return SocketType.TCP;
            } else {
                throw new Exception("Unknown SocketType " + s);
            }
        }

        public static string socketTypeToString(SocketType socketType) {
            switch (socketType) {
                case SocketType.UDP: {
                    return "UDP";
                }
                case SocketType.UDP_HEADER: {
                    return "UDP_HEADER";
                }
                case SocketType.TCP: {
                    return "TCP";
                }
                default: {
                    throw new Exception("Unknown SocketType " + socketType);
                }
            }
        }
    }
}
