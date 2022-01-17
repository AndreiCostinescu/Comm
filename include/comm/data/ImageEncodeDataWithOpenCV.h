//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 29-May-21.
//

#ifndef COMM_DATA_IMAGEENCODEDATAWITHOPENCV_H
#define COMM_DATA_IMAGEENCODEDATAWITHOPENCV_H

#include <comm/data/ImageDataWithOpenCV.h>
#include <comm/data/ImageEncodeData.h>
#include <opencv2/opencv.hpp>

namespace comm {
    class ImageEncodeDataWithOpenCV : public ImageEncodeData, public ImageDataWithOpenCV {
    public:
        ImageEncodeDataWithOpenCV();

        ImageEncodeDataWithOpenCV(cv::Mat image, int id, Encoding encoding);

        ImageEncodeDataWithOpenCV(const std::vector<unsigned char> &imageEncodedBytes, int imageHeight, int imageWidth,
                                  int imageType, int id, Encoding encoding);

        ~ImageEncodeDataWithOpenCV() override;

        bool serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        char *getDeserializeBuffer() override;

        bool deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;
    };
}

#endif //COMM_DATA_IMAGEENCODEDATAWITHOPENCV_H
