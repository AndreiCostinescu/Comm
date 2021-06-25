import cv2 as cv
import enum
import numpy as np
from comm.comm_data.ImageData import ImageData
from comm.comm_data.MessageType import MessageType
from comm.comm_utils.Buffer import Buffer
from comm.comm_utils.utils import memcpy
from typing import Optional


class ImageEncodeData(ImageData):
    headerSize = 24

    class Encoding(enum.Enum):
        JPEG = 0,
        PNG = 1,
        TIFF = 2

        @staticmethod
        def convertEncodingToInt(encoding: 'Encoding'):
            if encoding == ImageEncodeData.Encoding.JPEG:
                return 0
            elif encoding == ImageEncodeData.Encoding.PNG:
                return 1
            elif encoding == ImageEncodeData.Encoding.TIFF:
                return 2
            else:
                raise RuntimeError("Unknown encoding " + str(encoding))

        @staticmethod
        def convertIntToEncoding(encoding: int):
            if encoding == 0:
                return ImageEncodeData.Encoding.JPEG
            elif encoding == 1:
                return ImageEncodeData.Encoding.PNG
            elif encoding == 2:
                return ImageEncodeData.Encoding.TIFF
            else:
                raise RuntimeError("Unknown encoding " + str(encoding))

        @staticmethod
        def convertEncodingToString(encoding: 'Encoding'):
            if encoding == ImageEncodeData.Encoding.JPEG:
                return "jpg"
            elif encoding == ImageEncodeData.Encoding.PNG:
                return "png"
            elif encoding == ImageEncodeData.Encoding.TIFF:
                return "tiff"
            else:
                raise RuntimeError("Unknown encoding " + str(encoding))

        @staticmethod
        def convertStringToEncoding(encoding: str):
            if s == "jpeg":
                return ImageEncodeData.Encoding.JPEG
            elif s == "png":
                return ImageEncodeData.Encoding.PNG
            elif encoding == "tiff":
                return ImageEncodeData.Encoding.TIFF
            else:
                raise RuntimeError("Unknown Encoding " + encoding)

    def __init__(self, image: np.ndarray = None, id: int = -1, encoding: Encoding = None):
        super(ImageEncodeData, self).__init__(image, id)
        self.encodedImage = []  # type: np.ndarray
        self.encodedContentSize = 0
        self.encoding = encoding

    def getMessageType(self) -> MessageType:
        return MessageType.IMAGE_ENCODE

    def serialize(self, buffer: Buffer, start: int, forceCopy: bool, verbose: bool) -> bool:
        if self.serializeState == 0:
            buffer.setBufferContentSize(ImageData.headerSize)
            if verbose:
                print("Serialize: ", self.image.shape[0], ", ", self.image.shape[1], ", ", self.image.shape[2], ", ",
                      self.image.size)

            buffer.setInt(self.id, start)
            buffer.setInt(self.imageHeight, start + 4)
            buffer.setInt(self.imageWidth, start + 8)
            buffer.setInt(self.imageType, start + 12)

            retval, self.encodedImage = cv.imencode(
                "." + ImageEncodeData.Encoding.convertEncodingToString(self.encoding), self.image);
            self.encodedContentSize = len(self.encodedImage)

            buffer.setInt(self.encodedContentSize, start + 16)
            buffer.setInt(ImageEncodeData.Encoding.convertEncodingToInt(self.encoding), start + 20)

            if verbose:
                dataBuffer = buffer.getBuffer()
                print("Serialized content: ")
                for i in range(ImageData.headerSize):
                    print(int(dataBuffer[i]), "", end="")
                print()
            self.serializeState = 1
            return False
        elif self.serializeState == 1:
            buffer.setBufferContentSize(self.encodedContentSize)
            buffer.buffer = memcpy(buffer.getBuffer(), start, self.encodedImage.tobytes(), 0, self.encodedContentSize)
            self.serializeState = 0
            return True
        else:
            print("Impossible serialize state...", self.serializeState)
            self.resetSerializeState()
            return False

    def getExpectedDataSize(self) -> int:
        if self.deserializeState == 0:
            return ImageEncodeData.headerSize
        elif self.deserializeState == 1:
            return self.encodedContentSize
        else:
            raise RuntimeError("Impossible deserialize state... " + str(self.deserializeState))

    def getDeserializeBuffer(self) -> Optional[bytes]:
        return None

    def deserialize(self, buffer: Buffer, start: int, forceCopy: bool, verbose: bool) -> bool:
        if self.deserializeState == 0:
            self.imageDeserialized = False
            self.id = buffer.getInt(start)
            self.imageHeight = buffer.getInt(start + 4)
            self.imageWidth = buffer.getInt(start + 8)
            self.imageType = buffer.getInt(start + 12)
            self.encodedContentSize = buffer.getInt(start + 16)
            self.encoding = ImageEncodeData.Encoding.convertIntToEncoding(buffer.getInt(start + 20))

            # print("Deserializing image:", self.imageHeight, self.imageWidth, self.imageType, self.contentSize)
            self.deserializeState = 1
            return False
        elif self.deserializeState == 1:
            self.encodedImage = np.frombuffer(buffer.getBuffer(), dtype=np.int8)
            self.image = cv.imdecode(self.encodedImage, cv.IMREAD_COLOR)
            self.imageHeight = self.image.shape[0]
            self.imageWidth = self.image.shape[1]
            self.imageType = ImageData.imageCVType(self.image)
            self.imageDeserialized = True
            self.deserializeState = 0
            return True
        else:
            print("Impossible deserialize state...", self.deserializeState)
            self.resetSerializeState()
            return False

    def setEncoding(self, _encoding: Encoding):
        self.encoding = _encoding

    def getEncoding(self):
        return self.encoding
