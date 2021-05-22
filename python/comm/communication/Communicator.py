from comm.communication.Communication import Communication
from comm.comm_data.CommunicationData import CommunicationData
from comm.comm_data.DataCollection import DataCollection
from comm.comm_data.utils import createCommunicationData
from comm.comm_data.MessageType import MessageType
from comm.comm_data.StatusData import StatusData
from comm.comm_socket.SocketType import SocketType
from comm.comm_utils.Reference import Reference
from typing import Optional, Tuple, Callable


class Communicator:
    @staticmethod
    def send(comm: Communication, socketType: SocketType, data: CommunicationData or MessageType, retries: int = 0,
             verbose: bool = False) -> bool:
        if not data:
            return True
        if not comm.transmitData(socketType, data, False, True, retries, verbose):
            if comm.getErrorCode() > 0:
                print("When sending data... error: " + comm.getErrorString(), str(comm.getErrorCode()))
            return False
        return True

    @staticmethod
    def syphon(comm: Communication, socketType: SocketType, messageType: MessageType, data: Optional[CommunicationData],
               quitFlag: bool, retries: int = 0, verbose: bool = False,
               syphonRetries: int = 10) -> Tuple[bool, MessageType]:
        return Communicator._syphon(comm, socketType, messageType, data, (lambda: not quitFlag), retries, verbose,
                                    syphonRetries)

    @staticmethod
    def listen(comm: Communication, socketType: SocketType, _dataCollection: DataCollection, quitFlag: bool,
               retries: int = 0, verbose: bool = False) -> Tuple[bool, MessageType, DataCollection]:
        return Communicator._listen(comm, socketType, _dataCollection, (lambda: not quitFlag), retries, verbose)

    @staticmethod
    def listenFor(comm: Communication, socketType: SocketType, data: CommunicationData, quitFlag: bool,
                  timeoutResult: Reference = None, countIgnoreOtherMessages: int = -1, countIgnoreMessages: int = -1,
                  retries: int = 0, verbose: bool = False) -> Tuple[bool, CommunicationData]:
        return Communicator._listenFor(comm, socketType, data, (lambda: not quitFlag), timeoutResult,
                                       countIgnoreOtherMessages, countIgnoreMessages, retries, verbose)

    @staticmethod
    def __isReceiveErrorOk(errorCode: int, messageType: MessageType, nothingOk: bool) -> Tuple[bool, MessageType]:
        if errorCode < 0:
            # nothing received
            messageType = MessageType.NOTHING
            return nothingOk, messageType
        elif errorCode == 0:
            print("Socket closed.")
        else:
            print("An actual error occurred... " + str(errorCode))
        return False, messageType

    @staticmethod
    def _syphon(comm: Communication, socketType: SocketType, messageType: MessageType,
                data: Optional[CommunicationData], notQuit: Callable[[], bool], retries: int = 0, verbose: bool = False,
                syphonRetries: int = 10) -> Tuple[bool, MessageType]:
        if messageType == MessageType.NOTHING:
            return True, messageType
        if messageType in [MessageType.COORDINATE, MessageType.IMAGE, MessageType.STATUS]:
            if not data:
                data = createCommunicationData(messageType)
            localRetriesBeforeFail = 1 if (syphonRetries < 1) else syphonRetries
            while notQuit() and localRetriesBeforeFail > 0:
                if not comm.recvData(socketType, data, False, True, retries, verbose):
                    if comm.getErrorCode() == -1:
                        localRetriesBeforeFail -= 1
                        continue
                    # it shouldn't be possible to receive nothing if we already have received the header!!!
                    # set nothingOk to false!
                    return Communicator.__isReceiveErrorOk(comm.getErrorCode(), messageType, False)
                return True, messageType
            return False, messageType
        else:
            raise RuntimeError("Unknown message type: " + str(messageType))

    @staticmethod
    def _listen(comm: Communication, socketType: SocketType, _dataCollection: DataCollection,
                notQuit: Callable[[], bool], retries: int = 0,
                verbose: bool = False) -> Tuple[bool, MessageType, DataCollection]:
        recvSuccess, messageType = comm.recvMessageType(socketType, False, retries, verbose)
        if not recvSuccess:
            recvErrorOk, messageType = Communicator.__isReceiveErrorOk(comm.getErrorCode(), messageType, True)
            if recvErrorOk:
                return True, messageType, _dataCollection
            print("Error when receiving message type... setting \"quit\"")
            messageType = MessageType.STATUS
            status = _dataCollection.get(messageType)  # type: StatusData
            status.setCommand("quit")
            return True, messageType, _dataCollection

        data = _dataCollection.get(messageType)
        syphonResult, messageType = Communicator._syphon(comm, socketType, messageType, data, notQuit, retries, verbose)
        if not syphonResult:
            print("Error when syphoning data " + MessageType.messageTypeToString(messageType) + "... setting \"quit\"")
            messageType = MessageType.STATUS
            status = _dataCollection.get(messageType)  # type: StatusData
            status.setCommand("quit")

        return True, messageType, _dataCollection

    @staticmethod
    def _listenFor(comm: Communication, socketType: SocketType, data: CommunicationData, notQuit: Callable[[], bool],
                   timeoutResult: Reference = None, countIgnoreOtherMessages: int = -1, countIgnoreMessages: int = -1,
                   retries: int = 0, verbose: bool = False) -> Tuple[bool, CommunicationData]:
        assert data
        messageType = MessageType.NOTHING
        while notQuit():
            recvResult, messageType = comm.recvMessageType(socketType, False, retries, verbose)
            if not recvResult:
                recvErrorOk, messageType = Communicator.__isReceiveErrorOk(comm.getErrorCode(), messageType, True)
                if recvErrorOk:
                    continue
                return False, data

            if messageType != data.getMessageType():
                if messageType != MessageType.NOTHING:
                    print("Wrong messageType... expected", MessageType.messageTypeToString(data.getMessageType()),
                          "got", MessageType.messageTypeToString(messageType))
                syphonResult, messageType = Communicator._syphon(comm, socketType, messageType, None, notQuit)
                if syphonResult:
                    return False, data
                # it can happen that messageType becomes NOTHING because we receive noting in Communicator::syphon!
                if messageType != MessageType.NOTHING and countIgnoreOtherMessages > 0:
                    countIgnoreOtherMessages -= 1
                if countIgnoreMessages > 0:
                    countIgnoreMessages -= 1
                if countIgnoreOtherMessages == 0 or countIgnoreMessages == 0:
                    if timeoutResult is not None:
                        timeoutResult.value = True
                    return False, data
            else:
                break

        if not notQuit():
            return False, data

        while notQuit():
            syphonResult, messageType = Communicator._syphon(comm, socketType, messageType, data, notQuit, retries,
                                                             verbose)
            if not syphonResult:
                if comm.getErrorCode() < 0:
                    continue
                return False, data
            break

        return notQuit(), data
