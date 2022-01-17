//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 12.07.2021.
//

#include <comm/utils/Buffer.h>
#include <comm/data/DataCollectionWithOpenCV.h>
#include <comm/data/ImageEncodeDataWithOpenCV.h>
#include <fstream>
#include <iostream>

using namespace comm;
using namespace cv;
using namespace std;

void readBinaryFile(const string &path, char *&contentBuffer, int &contentSize) {
    char *buffer;
    long long size;
    ifstream file(path, ios::in | ios::binary | ios::ate);
    size = file.tellg();
    file.seekg(0, ios::beg);
    buffer = new char[size];
    file.read(buffer, size);
    file.close();

    cout << "the complete file is in a buffer";
    contentBuffer = buffer;
    contentSize = (int) size;
}

int main() {
    cout << "Hello World!" << endl;

    char *contentBuffer;
    int contentSize;
    readBinaryFile("../../data/Lena.png", contentBuffer, contentSize);
    vector<unsigned char> imageEncodedBytes(contentSize);
    for (int i = 0; i < contentSize; i++) {
        imageEncodedBytes[i] = contentBuffer[i];
    }
    DataCollectionWithOpenCV data;
    ImageEncodeDataWithOpenCV image, *decodedImage = dynamic_cast<ImageEncodeDataWithOpenCV *>(data.get(
            MessageType::IMAGE_ENCODE));
    ImageEncodeData::Encoding encoding = ImageEncodeData::Encoding::PNG;

    image.setImageEncodedBytes(imageEncodedBytes, 330, 330, (int) CV_8UC3, 0, encoding);
    cout << "Image size: " << image.getImage().rows << "x" << image.getImage().cols << ": " << image.getImage().type()
         << "; image bytes = " << image.getImageBytesSize() << endl;

    Buffer b;
    bool serializeDone, deserializeDone;

    int state = 0;
    cout << "Expecting message type: " << messageTypeToString(decodedImage->getMessageType()) << endl;
    while (true) {
        cout << "Serialize state: " << state << endl;
        serializeDone = image.serialize(&b, 0, false, true);
        deserializeDone = decodedImage->deserialize(&b, 0, false, true);
        cout << "Serialize state: " << state << ", buffer content size = " << b.getBufferContentSize() << endl;

        assert (deserializeDone == serializeDone);
        if (serializeDone) {
            break;
        }
        state++;
    }

    cout << "Encoded with " << ImageEncodeData::convertEncodingToString(image.getEncoding()) << " and decoded with "
         << ImageEncodeData::convertEncodingToString(decodedImage->getEncoding()) << endl;

    cv::imshow("Lena decoded", decodedImage->getImage());
    cv::waitKey(0);

    assert (decodedImage->getID() == image.getID());

    delete[] contentBuffer;
    cv::destroyAllWindows();
    return 0;
}
