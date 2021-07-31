//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 10.03.2021.
//

#ifndef COMM_DATA_IMAGEDATA_H
#define COMM_DATA_IMAGEDATA_H

#include <comm/data/CommunicationData.h>
#include <comm/utils/Buffer.h>

#ifdef WITH_OPENCV

#include <opencv2/opencv.hpp>

#else

typedef unsigned char uchar;

#endif

namespace comm {
    class ImageData : public CommunicationData {
    public:
        static const int headerSize;

        static int getOpenCVType(int imageChannels, int bytesPerElement, bool unsignedValue, bool floatValue);

        ImageData();

        #ifdef WITH_OPENCV

        ImageData(cv::Mat image, int id);

        #endif

        ImageData(uchar *imageBytes, int imageByteSize, int imageHeight, int imageWidth, int imageType, int id);

        MessageType getMessageType() override;

        bool serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        [[nodiscard]] int getExpectedDataSize() const override;

        char *getDeserializeBuffer() override;

        bool deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        void setID(int id);

        #ifdef WITH_OPENCV

        void setImage(cv::Mat image);

        [[nodiscard]] cv::Mat getImage() const;

        #endif

        void setImage(unsigned char *_imageBytes, int _imageByteSize, int _imageHeight, int _imageWidth,
                      int _imageType);

        [[nodiscard]] int getID() const;

        [[nodiscard]] int getHeight() const;

        [[nodiscard]] int getWidth() const;

        [[nodiscard]] int getType() const;

        [[nodiscard]] char *getImageBytes() const;

        [[nodiscard]] int getImageBytesSize() const;

        [[nodiscard]] bool isImageDeserialized() const;

    protected:
        #ifdef WITH_OPENCV
        cv::Mat image{};
        #endif
        uchar *imageBytes;
        int id, imageHeight, imageWidth, imageType, contentSize;
        bool imageDeserialized;
    };
}

#endif //COMM_DATA_IMAGEDATA_H
