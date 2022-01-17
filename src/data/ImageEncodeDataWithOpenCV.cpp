//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 29-May-21.
//

#include <comm/data/ImageEncodeDataWithOpenCV.h>
#include <comm/socket/utils.h>
#include <cassert>

using namespace comm;
using namespace std;

ImageEncodeDataWithOpenCV::ImageEncodeDataWithOpenCV() : ImageEncodeData() {}

ImageEncodeDataWithOpenCV::ImageEncodeDataWithOpenCV(cv::Mat image, int id, Encoding encoding) :
        ImageDataWithOpenCV(move(image), id), ImageEncodeData() {
    this->encoding = encoding;
}

ImageEncodeDataWithOpenCV::ImageEncodeDataWithOpenCV(const vector<unsigned char> &imageEncodedBytes, int imageHeight,
                                                     int imageWidth, int imageType, int id, Encoding encoding) :
        ImageDataWithOpenCV(nullptr, 0, imageHeight, imageWidth, imageType, id), ImageEncodeData() {
    this->encoding = encoding;
    this->encodedImage = imageEncodedBytes;
    this->encodedContentSize = (int) this->encodedImage.size();
}

ImageEncodeDataWithOpenCV::~ImageEncodeDataWithOpenCV() = default;

bool ImageEncodeDataWithOpenCV::serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) {
    switch (this->serializeState) {
        case 0: {
            buffer->setBufferContentSize(start + ImageEncodeData::headerSize);
            buffer->setInt(this->id, start);
            buffer->setInt(this->imageHeight, start + 4);
            buffer->setInt(this->imageWidth, start + 8);
            buffer->setInt(this->imageType, start + 12);

            if (this->imageBytes != nullptr) {
                cv::imencode("." + ImageEncodeData::convertEncodingToString(this->encoding), this->image,
                             this->encodedImage);
                this->encodedContentSize = (int) this->encodedImage.size();
            }

            buffer->setInt(this->encodedContentSize, start + 16);
            buffer->setInt(this->encoding, start + 20);
            if (verbose) {
                cout << "Serialize: " << this->imageHeight << ", " << this->imageWidth << ", " << this->imageType
                     << ", " << this->encodedContentSize << endl;
                char *dataBuffer = buffer->getBuffer();
                cout << "Serialized content: ";
                for (int i = 0; i < ImageEncodeData::headerSize; i++) {
                    cout << ((int) (unsigned char) dataBuffer[start + i]) << " ";
                }
                cout << endl;
            }
            this->serializeState = 1;
            return false;
        }
        case 1: {
            buffer->setBufferContentSize(this->encodedContentSize);
            char *writeBuffer = buffer->getBuffer();
            for (int i = 0; i < this->encodedContentSize; i++) {
                writeBuffer[i] = this->encodedImage[i];
            }
            this->serializeState = 0;
            return true;
        }
        default: {
            (*cerror) << "Impossible serialize state... " << this->serializeState << endl;
            this->resetSerializeState();
            return false;
        }
    }
}

char *ImageEncodeDataWithOpenCV::getDeserializeBuffer() {
    return ImageEncodeData::getDeserializeBuffer();
}

bool ImageEncodeDataWithOpenCV::deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) {
    switch (this->deserializeState) {
        case 0: {
            this->imageDeserialized = false;
            this->id = buffer->getInt(start);
            this->imageHeight = buffer->getInt(start + 4);
            this->imageWidth = buffer->getInt(start + 8);
            this->imageType = buffer->getInt(start + 12);
            this->encodedContentSize = buffer->getInt(start + 16);
            this->encoding = Encoding(buffer->getInt(start + 20));
            this->deserializeState = 1;
            return false;
        }
        case 1: {
            this->encodedImage.resize(this->encodedContentSize);
            assert((buffer->getBufferContentSize() - start) == this->encodedContentSize);

            char *encodedBuffer = buffer->getBuffer();
            for (int i = 0; i < this->encodedContentSize; i++) {
                this->encodedImage[i] = encodedBuffer[i];
            }
            // TODO: change the flag depending on the image type!
            cv::imdecode(this->encodedImage, cv::IMREAD_COLOR, &this->image);
            if (this->image.data != nullptr) {
                this->imageDeserialized = true;
            }

            this->deserializeState = 0;
            return true;
        }
        default: {
            (*cerror) << "Impossible deserialize state... " << this->deserializeState << endl;
            this->resetDeserializeState();
            return false;
        }
    }
}
