using System;

namespace Comm.data {
    public enum CommunicatorState {
        COMMUNICATOR_IDLE = 0,
        COMMUNICATOR_ACTIVE = 1,
        COMMUNICATOR_DONE = 2,
    };

    public static class CommunicatorStateConverter {
        public static CommunicatorState stringToCommunicatorState(string s) {
            if (s == "idle") {
                return CommunicatorState.COMMUNICATOR_IDLE;
            } else if (s == "active") {
                return CommunicatorState.COMMUNICATOR_ACTIVE;
            } else if (s == "done") {
                return CommunicatorState.COMMUNICATOR_DONE;
            }
            throw new Exception("Can not convert " + s + " to CommunicatorState enum");
        }

        public static string communicatorStateToString(CommunicatorState communicatorState) {
            switch (communicatorState) {
                case CommunicatorState.COMMUNICATOR_IDLE: {
                    return "idle";
                }
                case CommunicatorState.COMMUNICATOR_ACTIVE: {
                    return "active";
                }
                case CommunicatorState.COMMUNICATOR_DONE: {
                    return "done";
                }
                default: {
                    throw new Exception("Undefined CommunicatorState: " + communicatorState);
                }
            }
        }

        public static string convertCommunicatorStateToStatus(CommunicatorState communicatorState) {
            switch (communicatorState) {
                case CommunicatorState.COMMUNICATOR_IDLE: {
                    return "idle";
                }
                case CommunicatorState.COMMUNICATOR_ACTIVE: {
                    return "active";
                }
                case CommunicatorState.COMMUNICATOR_DONE: {
                    return "done";
                }
                default: {
                    throw new Exception("Unknown CommunicatorState " + communicatorState);
                }
            }
        }
    };


}
