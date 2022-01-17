//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 29-May-21.
//

#include <comm/data/DataCollectionWithOpenCV.h>
#include <comm/data/ImageEncodeDataWithOpenCV.h>
#include <iostream>

using namespace comm;
using namespace std;

int main() {
    cout << "Hello World!" << endl;

    cv::Mat lena = cv::imread("../../data/Lena.png");
    DataCollectionWithOpenCV data;
    ImageEncodeDataWithOpenCV image(lena, 0, ImageEncodeData::JPEG), *decodedImage;
    decodedImage = dynamic_cast<ImageEncodeDataWithOpenCV *>(data.get(MessageType::IMAGE_ENCODE));
    cout << "Image size: " << lena.rows << "x" << lena.cols << ": " << lena.type()
         << "; raw image bytes = " << image.getImageBytesSize() << endl;

    Buffer b;
    bool serializeDone, deserializeDone;
    for (int encoding = 0; encoding <= 2; encoding++) {
        int state = 0;
        cout << "Expecting message type: " << messageTypeToString(decodedImage->getMessageType()) << endl;
        image.setEncoding(ImageEncodeData::Encoding(encoding));
        while (true) {
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
        cout << "Different pixels in image per channel " << cv::sum(decodedImage->getImage() != image.getImage()) / 255
             << endl;
    }

    cout << "Test finished successfully!" << endl;

    return 0;
}
