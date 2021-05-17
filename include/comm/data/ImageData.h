//
// Created by ga78cat on 10.03.2021.
//

#ifndef PINAKOTHEKDRAWING_IMAGEDATA_H
#define PINAKOTHEKDRAWING_IMAGEDATA_H

#include <comm/data/CommunicationData.h>
#include <comm/utils/Buffer.h>
#include <opencv2/opencv.hpp>

namespace comm {
    class ImageData : public CommunicationData {
    public:
        static const int headerSize;

        ImageData();

        ImageData(cv::Mat image, int id);

        MessageType getMessageType() override;

        bool serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        [[nodiscard]] int getExpectedDataSize() const override;

        char *getDeserializeBuffer() override;

        bool deserialize(Buffer *buffer, int start, bool verbose) override;

        void setID(int id);

        void setImage(cv::Mat image, bool withSettingData = true);

        [[nodiscard]] cv::Mat getImage() const;

        [[nodiscard]] int getID() const;

        [[nodiscard]] int getHeight() const;

        [[nodiscard]] int getWidth() const;

        [[nodiscard]] int getType() const;

        [[nodiscard]] char *getImageBytes() const;

        [[nodiscard]] int getImageBytesSize() const;

        [[nodiscard]] bool isImageDeserialized() const;

    private:
        cv::Mat image;
        int id, imageHeight, imageWidth, imageType, contentSize;
        bool imageDeserialized;
    };
}

#endif //PINAKOTHEKDRAWING_IMAGEDATA_H
