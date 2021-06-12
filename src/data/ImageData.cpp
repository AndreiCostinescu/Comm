//
// Created by ga78cat on 10.03.2021.
//

#include <comm/data/ImageData.h>
#include <comm/socket/utils.h>
#include <iostream>
#include <utility>

using namespace comm;
using namespace std;
#ifdef WITH_OPENCV
using namespace cv;
#endif

const int ImageData::headerSize = 5 * sizeof(int);

int ImageData::getOpenCVType(int imageChannels, int bytesPerElement, bool unsignedValue, bool floatValue) {
    int cvType = 0;
    if (bytesPerElement == 1 && unsignedValue) {
        cvType = 0;
    } else if (bytesPerElement == 1) {
        cvType = 1;
    } else if (bytesPerElement == 2 && unsignedValue && !floatValue) {
        cvType = 2;
    } else if (bytesPerElement == 2 && !floatValue) {
        cvType = 3;
    } else if (bytesPerElement == 4 && !floatValue) {
        cvType = 4;
    } else if (bytesPerElement == 4) {
        cvType = 5;
    } else if (bytesPerElement == 8 && floatValue) {
        cvType = 6;
    } else if (bytesPerElement == 2 && floatValue) {
        cvType = 7;
    } else {
        throw runtime_error("Can not convert data (" + to_string(imageChannels) + ", " + to_string(bytesPerElement) +
                            ", " + to_string(unsignedValue) + ", " + to_string(floatValue) + ") to opencv type");
    }
    return cvType + ((imageChannels - 1) << 3);
}

ImageData::ImageData() : id(-1), imageHeight(0), imageWidth(0), imageType(0), contentSize(0), imageDeserialized(true),
                         imageBytes(nullptr) {}

ImageData::ImageData(uchar *imageBytes, int imageByteSize, int imageHeight, int imageWidth, int imageType, int id) :
        id(id), imageHeight(imageHeight), imageWidth(imageWidth), imageType(imageType), contentSize(imageByteSize),
        imageDeserialized(true), imageBytes(imageBytes) {}


#ifdef WITH_OPENCV

ImageData::ImageData(cv::Mat image, int id) : ImageData() {
    this->setID(id);
    this->setImage(std::move(image));
}

#endif

MessageType ImageData::getMessageType() {
    return MessageType::IMAGE;
}

bool ImageData::serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) {
    switch (this->serializeState) {
        case 0: {
            buffer->setBufferContentSize(start + ImageData::headerSize);
            if (verbose) {
                cout << "Serialize: " << this->imageHeight << ", " << this->imageWidth << ", " << this->imageType
                     << ", " << this->contentSize << endl;
            }
            buffer->setInt(this->id, start);
            buffer->setInt(this->imageHeight, start + 4);
            buffer->setInt(this->imageWidth, start + 8);
            buffer->setInt(this->imageType, start + 12);
            buffer->setInt(this->contentSize, start + 16);
            if (verbose) {
                char *dataBuffer = buffer->getBuffer();
                cout << "Serialized content: ";
                for (int i = 0; i < ImageData::headerSize; i++) {
                    cout << ((int) (unsigned char) dataBuffer[start + i]) << " ";
                }
                cout << endl;
            }
            this->serializeState = 1;
            return false;
        }
        case 1: {
            if (forceCopy) {
                buffer->setData(this->getImageBytes(), this->contentSize, start);
            } else {
                if (start != 0) {
                    throw runtime_error("Can not set a reference to data not starting at the first position!");
                }
                buffer->setConstReferenceToData(this->getImageBytes(), this->contentSize);
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
            #ifdef WITH_OPENCV
            this->image = Mat(this->imageHeight, this->imageWidth, this->imageType);
            return reinterpret_cast<char *>(this->image.data);
            #else
            return nullptr;
            #endif

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
            #ifdef WITH_OPENCV
            if (forceCopy) {
                // this->image = Mat(this->imageHeight, this->imageWidth, this->imageType);
                memcpy(this->image.data, buffer->getBuffer() + start, this->contentSize);
                this->imageBytes = this->image.data;
            }
            #else
            prepareBuffer((char *&) this->imageBytes, this->contentSize);
            memcpy(this->imageBytes, buffer->getBuffer() + start, this->contentSize);
            #endif
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

#ifdef WITH_OPENCV

void ImageData::setImage(Mat _image) {
    this->image = std::move(_image);
    this->imageBytes = this->image.data;
    this->imageHeight = this->image.rows;
    this->imageWidth = this->image.cols;
    this->imageType = this->image.type();
    this->contentSize = (int) this->image.step[0] * this->imageHeight;
}

Mat ImageData::getImage() const {
    return this->image;
}

#endif

void ImageData::setImage(unsigned char *_imageBytes, int _imageByteSize, int _imageHeight, int _imageWidth,
                         int _imageType) {
    this->imageBytes = _imageBytes;
    this->imageHeight = _imageHeight;
    this->imageWidth = _imageWidth;
    this->imageType = _imageType;
    this->contentSize = _imageByteSize;
    #if WITH_OPENCV
    this->image = this->image = Mat(this->imageHeight, this->imageWidth, this->imageType);
    memcpy(this->image.data, this->imageBytes, this->contentSize);
    #endif
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
    return reinterpret_cast<char *>(this->imageBytes);
}

int ImageData::getImageBytesSize() const {
    return this->contentSize;
}

bool ImageData::isImageDeserialized() const {
    return this->imageDeserialized;
}
