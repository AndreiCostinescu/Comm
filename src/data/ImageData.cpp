//
// Created by ga78cat on 10.03.2021.
//

#include <comm/data/ImageData.h>
#include <comm/socket/utils.h>
#include <iostream>
#include <utility>

using namespace comm;
using namespace cv;
using namespace std;

const int ImageData::headerSize = 5 * sizeof(int);

ImageData::ImageData()
        : image(), id(-1), imageHeight(0), imageWidth(0), imageType(0), contentSize(0), imageDeserialized(true) {}

ImageData::ImageData(cv::Mat image, int id) : ImageData() {
    this->setID(id);
    this->setImage(std::move(image));
}

MessageType ImageData::getMessageType() {
    return MessageType::IMAGE;
}

bool ImageData::serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) {
    switch (this->serializeState) {
        case 0: {
            buffer->setBufferContentSize(start + ImageData::headerSize);
            if (verbose) {
                cout << "Serialize: " << this->image.rows << ", " << this->image.cols << ", " << this->image.type()
                     << ", " << this->contentSize << endl;
            }
            buffer->setInt(this->id, start);
            buffer->setInt(this->image.rows, start + 4);
            buffer->setInt(this->image.cols, start + 8);
            buffer->setInt(this->image.type(), start + 12);
            buffer->setInt(this->contentSize, start + 16);
            if (verbose) {
                char *dataBuffer = buffer->getBuffer();
                cout << "Serialized content: ";
                for (int i = 0; i < ImageData::headerSize; i++) {
                    cout << ((int) dataBuffer[start + i]) << " ";
                }
                cout << endl;
            }
            this->serializeState = 1;
            return false;
        }
        case 1: {
            if (forceCopy) {
                buffer->setData((char *) this->image.data, this->contentSize, start);
            } else {
                if (start != 0) {
                    throw runtime_error("Can not set a reference to data not starting at the first position!");
                }
                buffer->setConstReferenceToData((const char *) this->image.data, this->contentSize);
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

int ImageData::getExpectedDataSize() const {
    switch (this->deserializeState) {
        case 0: {
            return ImageData::headerSize;
        }
        case 1: {
            return this->contentSize;
        }
        default : {
            throw runtime_error("Impossible deserialize state... " + to_string(this->deserializeState));
        }
    }
}

char *ImageData::getDeserializeBuffer() {
    switch (this->deserializeState) {
        case 0: {
            return nullptr;
        }
        case 1: {
            this->image = Mat(this->imageHeight, this->imageWidth, this->imageType);
            return reinterpret_cast<char *>(this->image.data);
        }
        default : {
            throw runtime_error("Impossible deserialize state... " + to_string(this->deserializeState));
        }
    }
}

bool ImageData::deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) {
    switch (this->deserializeState) {
        case 0: {
            this->imageDeserialized = false;
            this->id = buffer->getInt(start);
            this->imageHeight = buffer->getInt(start + 4);
            this->imageWidth = buffer->getInt(start + 8);
            this->imageType = buffer->getInt(start + 12);
            this->contentSize = buffer->getInt(start + 16);
            this->deserializeState = 1;
            return false;
        }
        case 1: {
            if (forceCopy) {
                // this->image = Mat(this->imageHeight, this->imageWidth, this->imageType);
                memcpy(this->image.data, buffer->getBuffer() + start, this->contentSize);
            }
            this->imageDeserialized = true;
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

void ImageData::setID(int _id) {
    this->id = _id;
}

void ImageData::setImage(Mat _image, bool withSettingData) {
    this->image = std::move(_image);
    if (withSettingData) {
        this->imageHeight = this->image.rows;
        this->imageWidth = this->image.cols;
        this->imageType = this->image.type();
        this->contentSize = (int) this->image.step[0] * this->imageHeight;
    }
}

Mat ImageData::getImage() const {
    return this->image;
}

int ImageData::getID() const {
    return this->id;
}

int ImageData::getHeight() const {
    return this->imageHeight;
}

int ImageData::getWidth() const {
    return this->imageWidth;
}

int ImageData::getType() const {
    return this->imageType;
}

char *ImageData::getImageBytes() const {
    return reinterpret_cast<char *>(this->image.data);
}

int ImageData::getImageBytesSize() const {
    return this->contentSize;
}

bool ImageData::isImageDeserialized() const {
    return this->imageDeserialized;
}
