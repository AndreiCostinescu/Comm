import enum


class CommunicatorState(enum.Enum):
    COMMUNICATOR_IDLE = 0,
    COMMUNICATOR_ACTIVE = 1,
    COMMUNICATOR_DONE = 2,

    @staticmethod
    def stringToCommunicatorMethod(s: str):
        if s == "idle":
            return CommunicatorState.COMMUNICATOR_IDLE
        elif s == "active":
            return CommunicatorState.COMMUNICATOR_ACTIVE
        elif s == "done":
            return CommunicatorState.COMMUNICATOR_DONE
        else:
            raise RuntimeError("Unknown CommunicatorState " + s)

    @staticmethod
    def communicatorStateToString(communicatorState: 'CommunicatorState'):
        if communicatorState == CommunicatorState.COMMUNICATOR_IDLE:
            return "idle"
        elif communicatorState == CommunicatorState.COMMUNICATOR_ACTIVE:
            return "active"
        elif communicatorState == CommunicatorState.COMMUNICATOR_DONE:
            return "done"
        else:
            raise RuntimeError("Unknown CommunicatorState " + str(communicatorState))

    @staticmethod
    def convertCommunicatorStateToStatus(communicatorState: 'CommunicatorState'):
        if communicatorState == CommunicatorState.COMMUNICATOR_IDLE:
            return "idle"
        elif communicatorState == CommunicatorState.COMMUNICATOR_ACTIVE:
            return "active"
        elif communicatorState == CommunicatorState.COMMUNICATOR_DONE:
            return "done"
        else:
            raise RuntimeError("Unknown CommunicatorState " + str(communicatorState))
