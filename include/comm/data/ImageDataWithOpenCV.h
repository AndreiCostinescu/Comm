//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 10.03.2021.
//

#ifndef COMM_DATA_IMAGEDATAWITHOPENCV_H
#define COMM_DATA_IMAGEDATAWITHOPENCV_H

#include <comm/data/ImageData.h>
#include <opencv2/opencv.hpp>

namespace comm {
    class ImageDataWithOpenCV : public virtual ImageData {
    public:
        ImageDataWithOpenCV();

        ImageDataWithOpenCV(cv::Mat image, int id);

        ImageDataWithOpenCV(unsigned char *imageBytes, int imageByteSize, int imageHeight, int imageWidth, int imageType, int id);

        ~ImageDataWithOpenCV() override;

        char *getDeserializeBuffer() override;

        bool deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        void setImage(cv::Mat image);

        [[nodiscard]] cv::Mat getImage() const;

        void setImage(unsigned char *_imageBytes, int _imageByteSize, int _imageHeight, int _imageWidth, int _imageType) override;

    protected:
        cv::Mat image{};
    };
}

#endif //COMM_DATA_IMAGEDATAWITHOPENCV_H
