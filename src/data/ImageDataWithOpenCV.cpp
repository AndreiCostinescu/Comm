//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 10.03.2021.
//

#include <comm/data/ImageDataWithOpenCV.h>
#include <comm/socket/utils.h>
#include <iostream>
#include <utility>

using namespace comm;
using namespace cv;
using namespace std;

ImageDataWithOpenCV::ImageDataWithOpenCV() : ImageData() {}

ImageDataWithOpenCV::ImageDataWithOpenCV(unsigned char *imageBytes, int imageByteSize, int imageHeight, int imageWidth,
                                         int imageType, int id) :
        ImageData(imageBytes, imageByteSize, imageHeight, imageWidth, imageType, id) {}


ImageDataWithOpenCV::ImageDataWithOpenCV(cv::Mat image, int id) : ImageData() {
    this->setID(id);
    this->setImage(std::move(image));
}

ImageDataWithOpenCV::~ImageDataWithOpenCV() = default;

char *ImageDataWithOpenCV::getDeserializeBuffer() {
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

bool ImageDataWithOpenCV::deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) {
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
                this->imageBytes = this->image.data;
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

void ImageDataWithOpenCV::setImage(Mat _image) {
    this->image = std::move(_image);
    this->imageBytes = this->image.data;
    this->imageHeight = this->image.rows;
    this->imageWidth = this->image.cols;
    this->imageType = this->image.type();
    this->contentSize = (int) this->image.step[0] * this->imageHeight;
}

Mat ImageDataWithOpenCV::getImage() const {
    return this->image;
}

void ImageDataWithOpenCV::setImage(unsigned char *_imageBytes, int _imageByteSize, int _imageHeight, int _imageWidth,
                         int _imageType) {
    ImageData::setImage(_imageBytes, _imageByteSize, _imageHeight, _imageWidth, _imageType);
    if (this->imageBytes != nullptr) {
        this->image = Mat(this->imageHeight, this->imageWidth, this->imageType);
        memcpy(this->image.data, this->imageBytes, this->contentSize);
    }
}
