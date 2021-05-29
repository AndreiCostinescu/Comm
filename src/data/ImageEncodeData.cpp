//
// Created by Andrei on 29-May-21.
//

#include <comm/data/ImageEncodeData.h>
#include <utility>

using namespace comm;
using namespace std;

const int ImageEncodeData::headerSize = 6 * sizeof(int);

string ImageEncodeData::convertEncodingToString(Encoding encoding) {
    switch (encoding) {
        case JPEG: {
            return "jpg";
        }
        case PNG: {
            return "png";
        }
        case TIFF: {
            return "tiff";
        }
        default : {
            throw runtime_error("Unknown encoding " + to_string(encoding));
        }
    }
}

ImageEncodeData::Encoding ImageEncodeData::convertStringToEncoding(const string &encoding) {
    if (encoding == "jpg") {
        return Encoding::JPEG;
    } else if (encoding == "png") {
        return Encoding::PNG;
    } else if (encoding == "tiff") {
        return Encoding::TIFF;
    } else {
        throw runtime_error("Unknown encoding " + encoding);
    }
}

ImageEncodeData::ImageEncodeData() : ImageData(), encoding(Encoding::PNG), encodedContentSize(0), encodedImage() {}

ImageEncodeData::ImageEncodeData(cv::Mat image, int id, Encoding encoding) :
        ImageData(std::move(image), id), encoding(encoding), encodedContentSize(0), encodedImage() {}

MessageType ImageEncodeData::getMessageType() {
    return MessageType::IMAGE_ENCODE;
}

bool ImageEncodeData::serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) {
    switch (this->serializeState) {
        case 0: {
            buffer->setBufferContentSize(start + ImageEncodeData::headerSize);
            if (verbose) {
                cout << "Serialize: " << this->image.rows << ", " << this->image.cols << ", " << this->image.type()
                     << ", " << this->encodedContentSize << endl;
            }
            buffer->setInt(this->id, start);
            buffer->setInt(this->image.rows, start + 4);
            buffer->setInt(this->image.cols, start + 8);
            buffer->setInt(this->image.type(), start + 12);

            cv::imencode("." + ImageEncodeData::convertEncodingToString(this->encoding), this->image,
                         this->encodedImage);
            this->encodedContentSize = this->encodedImage.size();

            buffer->setInt(this->encodedContentSize, start + 16);
            buffer->setInt(this->encoding, start + 20);
            if (verbose) {
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

int ImageEncodeData::getExpectedDataSize() const {
    switch (this->deserializeState) {
        case 0: {
            return ImageEncodeData::headerSize;
        }
        case 1: {
            return this->encodedContentSize;
        }
        default : {
            throw runtime_error("Impossible deserialize state... " + to_string(this->deserializeState));
        }
    }
}

char *ImageEncodeData::getDeserializeBuffer() {
    return nullptr;
}

bool ImageEncodeData::deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) {
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
            assert ((buffer->getBufferContentSize() - start) == this->encodedContentSize);

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

void ImageEncodeData::setEncoding(Encoding _encoding) {
    this->encoding = _encoding;
}

ImageEncodeData::Encoding ImageEncodeData::getEncoding() {
    return this->encoding;
}