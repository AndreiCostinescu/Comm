//
// Created by Andrei on 30-May-21.
//

#include <comm/communication/Communication.h>
#include <comm/data/ImageData.h>

#include <utility>

using namespace comm;
using namespace cv;
using namespace std;

SocketType socketType = SocketType::UDP;

void rawSendImage(Communication &sender, Mat image, int prematureStop = 0, bool withHeader = true) {
    ImageData sendData(std::move(image), 0);
    Buffer sendBuffer;

    if (withHeader) {
        // image header
        SerializationHeader header(0, 0, ImageData::headerSize, false);
        sendBuffer.setInt(header.getInt(false), 0);
        assert (!sendData.serialize(&sendBuffer, 4, true, false));
        sender.sendRaw(socketType, sendBuffer.getBuffer(), sendBuffer.getBufferContentSize());
        cout << "Sent image header!" << endl;
    }

    // image data
    sendBuffer.setChar(1, 0);
    const int cutSize = 65500 - 4;
    int imageSize = sendData.getImageBytesSize(), copySize, cuts = (imageSize + cutSize - 1) / cutSize;
    char *imageData = sendData.getImageBytes();
    for (int iteration = 0; iteration < cuts - prematureStop; iteration++) {
        cout << "At send iteration " << iteration << endl;
        copySize = min(imageSize, cutSize);
        sendBuffer.setChar((char) iteration, 1);
        sendBuffer.setShort((short) copySize, 2);
        sendBuffer.setBufferContentSize(4 + copySize);

        memcpy(sendBuffer.getBuffer() + 4, imageData + sendData.getImageBytesSize() - imageSize, copySize);

        sender.sendRaw(socketType, sendBuffer.getBuffer(), sendBuffer.getBufferContentSize());
        imageSize -= copySize;
    }
    sendData.resetSerializeState();
    cout << "Finished sending!" << endl;
}

int main() {
    cout << "Hello World!" << endl;

    Mat lena = imread("../../data/Lena.png");
    cout << "Lena: " << lena.rows << " x " << lena.cols << endl;

    Communication sender, receiver;
    sender.createSocket(socketType, SocketPartner("127.0.0.1", 8400, false));
    receiver.createSocket(socketType, nullptr, 8400, 2000, 50);
    ImageData recvData;

    rawSendImage(sender, lena, 2);
    rawSendImage(sender, lena, 0, false);
    rawSendImage(sender, lena, 0);
    assert (!receiver.recvData(socketType, &recvData, true, true, false, 0, true) && receiver.getErrorCode() == -2);
    assert (!recvData.isImageDeserialized());
    cout << "Finished first receive!" << endl;
    assert (receiver.recvData(socketType, &recvData, true, true, false, 0, true));
    assert (recvData.isImageDeserialized());
    imshow("Received Image", recvData.getImage());
    waitKey(0);
    cout << "\n\n\n\n";

    rawSendImage(sender, lena);
    assert (receiver.recvData(socketType, &recvData, true, false, false, 0, true));
    assert (recvData.isImageDeserialized());
    imshow("Received Image", recvData.getImage());
    waitKey(0);
    cout << "\n\n\n\n";

    rawSendImage(sender, lena, 1);
    assert (!receiver.recvData(socketType, &recvData, true, false, false, 0, true) && receiver.getErrorCode() == -1);
    assert (!recvData.isImageDeserialized());
    // imshow("Received Image", recvData.getImage());
    // waitKey(0);
    cout << "\n\n\n\n";

    rawSendImage(sender, lena, 0);
    assert (receiver.recvData(socketType, &recvData, true, false, false, 0, true));
    assert (recvData.isImageDeserialized());
    imshow("Received Image", recvData.getImage());
    waitKey(0);
    cout << "\n\n\n\n";

    rawSendImage(sender, lena, 2);
    rawSendImage(sender, lena, 0);
    assert (!receiver.recvData(socketType, &recvData, true, false, false, 0, true) && receiver.getErrorCode() == -2);
    assert (!recvData.isImageDeserialized());
    assert (receiver.recvData(socketType, &recvData, true, false, false, 0, true));
    assert (recvData.isImageDeserialized());
    imshow("Received Image", recvData.getImage());
    waitKey(0);
    cout << "\n\n\n\n";

    rawSendImage(sender, lena, 0);
    rawSendImage(sender, lena, 0);
    rawSendImage(sender, lena, 0);
    assert (receiver.recvData(socketType, &recvData, true, false, false, 0, true));
    assert (recvData.isImageDeserialized());
    assert (receiver.recvData(socketType, &recvData, true, false, false, 0, true));
    assert (recvData.isImageDeserialized());
    assert (receiver.recvData(socketType, &recvData, true, false, false, 0, true));
    assert (recvData.isImageDeserialized());
    imshow("Received Image", recvData.getImage());
    waitKey(0);
    cout << "\n\n\n\n";

    rawSendImage(sender, lena, 0);
    rawSendImage(sender, lena, 2);
    rawSendImage(sender, lena, 0, false);
    rawSendImage(sender, lena, 0);
    assert (receiver.recvData(socketType, &recvData, true, false, false, 0, true));
    assert (recvData.isImageDeserialized());
    assert (!receiver.recvData(socketType, &recvData, true, false, false, 0, true) && receiver.getErrorCode() == -2);
    assert (!recvData.isImageDeserialized());
    assert (!receiver.recvData(socketType, &recvData, true, false, false, 0, true) && receiver.getErrorCode() == -2);
    assert (!recvData.isImageDeserialized());
    while (true) {
        bool recvResult = receiver.recvData(socketType, &recvData, true, false, false, 0, true);
        if (recvResult) {
            break;
        } else {
            assert (receiver.getErrorCode() == -2);
        }
    }
    imshow("Received Image", recvData.getImage());
    waitKey(0);
    cout << "\n\n\n\n";

    return 0;
}