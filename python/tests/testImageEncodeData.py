import cv2 as cv
import numpy as np
from comm.comm_data.DataCollection import DataCollection
from comm.comm_data.ImageEncodeData import ImageEncodeData
from comm.comm_data.MessageType import MessageType
from comm.comm_utils.Buffer import Buffer


def test():
    lena = cv.imread("/home/andrei/CLionProjects/Comm/data/Lena.png")
    print(lena.shape)
    image = ImageEncodeData(lena, 0, ImageEncodeData.Encoding.JPEG)
    data = DataCollection()
    decodedImage = data.get(MessageType.IMAGE_ENCODE)  # type: ImageEncodeData
    b = Buffer()

    for encoding in [ImageEncodeData.Encoding.JPEG, ImageEncodeData.Encoding.PNG, ImageEncodeData.Encoding.TIFF]:
        state = 0
        print("Expecting message type:", MessageType.messageTypeToString(decodedImage.getMessageType()))
        image.setEncoding(encoding)
        while True:
            serializeDone = image.serialize(b, 0, False, True)
            deserializeDone = decodedImage.deserialize(b, 0, False, False)
            print("Serialize state:", state, ", buffer content size =", b.getBufferContentSize())

            assert (deserializeDone == serializeDone);
            if serializeDone:
                break
            state = state + 1

        print("Encoded with", ImageEncodeData.Encoding.convertEncodingToString(image.getEncoding()), "and decoded with",
              ImageEncodeData.Encoding.convertEncodingToString(decodedImage.getEncoding()))

        cv.imshow("Lena decoded", decodedImage.getImage())
        cv.waitKey(0);

        assert (decodedImage.getID() == image.getID())
        print("Different pixels in image per channel", np.sum(np.sum(decodedImage.getImage() != image.getImage(), axis=0), axis=0))

    print("Test finished successfully!")


if __name__ == "__main__":
    test()
