from comm.communication.Communication import Communication
from comm.comm_data.CommunicationData import CommunicationData
from comm.comm_data.CommunicatorState import CommunicatorState
from comm.comm_data.DataCollection import DataCollection
from comm.comm_data.utils import createCommunicationData
from comm.comm_data.MessageType import MessageType
from comm.comm_data.StatusData import StatusData
from comm.comm_socket.SocketType import SocketType
from typing import Optional, Tuple


class Communicator:
    def __init__(self):
        self.quit = False
        self.state = CommunicatorState.COMMUNICATOR_IDLE
        self.dataCollection = DataCollection()

    def main(self):
        while not self.quit:
            self._preMain()
            try:
                self._main()
            except Exception as e:
                print("Exception caught: " + str(e))
            self._postMain()

    def stop(self):
        self.quit = True

    @staticmethod
    def isReceiveErrorOk(errorCode: int, messageType: MessageType, nothingOk: bool) -> Tuple[bool, MessageType]:
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
    def send(comm: Communication, socketType: SocketType, data: CommunicationData or MessageType, retries: int = 0,
             verbose: bool = False) -> bool:
        if isinstance(data, CommunicationData):
            if not data:
                return True
            if not comm.sendData(socketType, data, True, retries, verbose):
                if comm.getErrorCode() > 0:
                    print("When sending data... error: " + comm.getErrorString(), str(comm.getErrorCode()))
                return False
            return True
        else:
            assert isinstance(data, MessageType)
            if not comm.sendMessageType(socketType, data, retries, verbose):
                if comm.getErrorCode() > 0:
                    print("When sending data... error: " + comm.getErrorString(), comm.getErrorCode())
                    return False
            return True

    def syphon(self, comm: Communication, socketType: SocketType, messageType: MessageType,
               data: Optional[CommunicationData], retries: int = 0, verbose: bool = False,
               syphonRetries: int = 10) -> Tuple[bool, MessageType]:
        if messageType == MessageType.NOTHING:
            return True, messageType
        if messageType in [MessageType.COORDINATE, MessageType.IMAGE, MessageType.STATUS]:
            if not data:
                data = createCommunicationData(messageType)
            localRetriesBeforeFail = 1 if (syphonRetries < 1) else syphonRetries
            while not self.quit and localRetriesBeforeFail > 0:
                if not comm.recvData(socketType, data, retries, verbose):
                    if comm.getErrorCode() == -1:
                        localRetriesBeforeFail -= 1
                        continue
                    # it shouldn't be possible to receive nothing if we already have received the header!!!
                    # set nothingOk to false!
                    return Communicator.isReceiveErrorOk(comm.getErrorCode(), messageType, False)
                return True, messageType
            return False, messageType
        else:
            raise RuntimeError("Unknown message type: " + str(messageType))

    def listen(self, comm: Communication, socketType: SocketType, _dataCollection: DataCollection) \
            -> Tuple[bool, MessageType, DataCollection]:
        recvSuccess, messageType = comm.recvMessageType(socketType)
        if not recvSuccess:
            recvErrorOk, messageType = Communicator.isReceiveErrorOk(comm.getErrorCode(), messageType, True)
            if recvErrorOk:
                return True, messageType, _dataCollection
            print("Error when receiving message type... setting \"quit\"")
            messageType = MessageType.STATUS
            status = _dataCollection.get(messageType)  # type: StatusData
            status.setCommand("quit")
            return True, messageType, _dataCollection

        data = _dataCollection.get(messageType)
        syphonResult, messageType = self.syphon(comm, socketType, messageType, data)
        if not syphonResult:
            print("Error when syphoning data " + MessageType.messageTypeToString(messageType) + "... setting \"quit\"")
            messageType = MessageType.STATUS
            status = _dataCollection.get(messageType)  # type: StatusData
            status.setCommand("quit")

        return True, messageType, _dataCollection

    def listenFor(self, comm: Communication, socketType: SocketType, data: CommunicationData,
                  countIgnoreOtherMessages: int = -1, countIgnoreMessages: int = -1) -> Tuple[bool, CommunicationData]:
        assert data
        messageType = MessageType.NOTHING
        while not self.quit:
            recvResult, messageType = comm.recvMessageType(socketType)
            if not recvResult:
                recvErrorOk, messageType = Communicator.isReceiveErrorOk(comm.getErrorCode(), messageType, True)
                if recvErrorOk:
                    continue
                return False, data

            if messageType != data.getMessageType():
                if messageType != MessageType.NOTHING:
                    print("Wrong messageType... expected", MessageType.messageTypeToString(data.getMessageType()),
                          "got", MessageType.messageTypeToString(messageType))
                syphonResult, messageType = self.syphon(comm, socketType, messageType, None)
                if syphonResult:
                    return False, data
                # it can happen that messageType becomes NOTHING because we receive noting in Communicator::syphon!
                if messageType != MessageType.NOTHING and countIgnoreOtherMessages > 0:
                    countIgnoreOtherMessages -= 1
                if countIgnoreMessages > 0:
                    countIgnoreMessages -= 1
                if countIgnoreOtherMessages == 0 or countIgnoreMessages == 0:
                    return False, data
            else:
                break

        if self.quit:
            return False, data

        while not self.quit:
            syphonResult, messageType = self.syphon(comm, socketType, messageType, data)
            if not syphonResult:
                if comm.getErrorCode() < 0:
                    continue
                return False, data
            break

        return not self.quit, data

    def _preMain(self):
        pass

    def _main(self):
        pass

    def _postMain(self):
        pass
