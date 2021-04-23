import numpy as np
from python.comm.data.CommunicationData import CommunicationData
from python.comm.data.MessageType import MessageType
from python.comm.socket.Buffer import Buffer


class ImageData(CommunicationData):
    @staticmethod
    def npTypeToCVType(npType: np.dtype) -> int:
        if npType == np.uint8:
            cvType = 0
        elif npType == np.int8:
            cvType = 1
        elif npType == np.uint16:
            cvType = 2
        elif npType == np.int16:
            cvType = 3
        elif npType == np.int32:
            cvType = 4
        elif npType == np.float32:
            cvType = 5
        elif npType == np.float64:
            cvType = 6
        else:
            assert npType == np.float16
            cvType = 7
        return cvType

    @staticmethod
    def cvTypeToNPType(cvType: int) -> np.dtype:
        cvType &= 7

        if cvType == 0:
            npType = np.uint8
        elif cvType == 1:
            npType = np.int8
        elif cvType == 2:
            npType = np.uint16
        elif cvType == 3:
            npType = np.int16
        elif cvType == 4:
            npType = np.int32
        elif cvType == 5:
            npType = np.float32
        elif cvType == 6:
            npType = np.float64
        else:
            assert cvType == 7
            npType = np.float16
        return npType

    @staticmethod
    def imageCVType(image: np.ndarray):
        dim = image.shape[2] if len(image.shape) > 2 else 1
        cvType = ImageData.npTypeToCVType(image.dtype)
        return cvType + ((dim - 1) << 3)

    @staticmethod
    def imageDim(imageType: int) -> int:
        return imageType >> 3

    headerSize = 20

    def __init__(self, image: np.ndarray = None, _id: int = -1):
        super().__init__()
        self.id = _id
        if image is None:
            self.image = np.array([])  # type: np.ndarray
            self.imageHeight = 0
            self.imageWidth = 0
            self.imageType = 0
            self.contentSize = 0
            self.imageDeserialized = True
        else:
            self.setImage(image)

    def getMessageType(self) -> MessageType:
        return MessageType.IMAGE

    def serialize(self, buffer: Buffer, verbose: bool) -> bool:
        if self.serializeState == 0:
            buffer.setBufferContentSize(ImageData.headerSize)
            if verbose:
                print("Serialize: ", self.image.shape[0], ", ", self.image.shape[1], ", ", self.image.shape[2], ", ",
                      self.image.size)

            buffer.setInt(self.id, 0)
            buffer.setInt(self.imageHeight, 4)
            buffer.setInt(self.imageWidth, 8)
            buffer.setInt(self.imageType, 12)
            buffer.setInt(self.contentSize, 16)
            if verbose:
                dataBuffer = buffer.getBuffer()
                print("Serialized content: ")
                for i in range(ImageData.headerSize):
                    print(int(dataBuffer[i]), "")
                print()
            self.serializeState = 1
            return False
        elif self.serializeState == 1:
            buffer.setReferenceToData(self.image.tobytes(), self.contentSize)
            self.serializeState = 0
            return True
        else:
            print("Impossible serialize state...", self.serializeState)
            self.resetSerializeState()
            return False

    def getExpectedDataSize(self) -> int:
        if self.deserializeState == 0:
            return ImageData.headerSize
        elif self.deserializeState == 1:
            return self.contentSize
        else:
            raise RuntimeError("Impossible deserialize state... " + str(self.deserializeState))

    def deserialize(self, buffer: Buffer, start: int, verbose: bool) -> bool:
        if self.deserializeState == 0:
            self.imageDeserialized = False
            self.id = buffer.getInt(start)
            self.imageHeight = buffer.getInt(start + 4)
            self.imageWidth = buffer.getInt(start + 8)
            self.imageType = buffer.getInt(start + 12)
            self.contentSize = buffer.getInt(start + 16)
            self.deserializeState = 1
            return False
        elif self.deserializeState == 1:
            self.image = np.frombuffer(buffer.getBuffer(), dtype=ImageData.cvTypeToNPType(self.imageType)).reshape(
                [self.imageHeight, self.imageWidth, ImageData.imageDim(self.imageType)])
            self.imageDeserialized = True
            self.deserializeState = 0
            return True
        else:
            print("Impossible deserialize state...", self.deserializeState)
            self.resetSerializeState()
            return False

    def setID(self, _id):
        self.id = _id

    def setImage(self, _image: np.ndarray, withSettingData: bool = True):
        self.image = _image
        if withSettingData:
            self.imageHeight = self.image.shape[0]
            self.imageWidth = self.image.shape[1]
            self.imageType = ImageData.imageCVType(self.image)
            self.contentSize = self.image.size

    def getImage(self) -> np.ndarray:
        return self.image

    def getID(self) -> int:
        return self.id

    def getHeight(self) -> int:
        return self.imageHeight

    def getWidth(self) -> int:
        return self.imageWidth

    def getType(self) -> int:
        return self.imageType

    def getImageBytes(self) -> bytes:
        return self.image.tobytes()

    def getImageBytesSize(self) -> int:
        return self.contentSize

    def isImageDeserialized(self) -> bool:
        return self.imageDeserialized
