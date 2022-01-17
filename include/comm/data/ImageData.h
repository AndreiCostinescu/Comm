//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 10.03.2021.
//

#ifndef COMM_DATA_IMAGEDATA_H
#define COMM_DATA_IMAGEDATA_H

#include <comm/data/CommunicationData.h>
#include <comm/utils/Buffer.h>

namespace comm {
    class ImageData : public virtual CommunicationData {
    public:
        static const int headerSize;

        static int getOpenCVType(int imageChannels, int bytesPerElement, bool unsignedValue, bool floatValue);

        ImageData();

        ImageData(unsigned char *imageBytes, int imageByteSize, int imageHeight, int imageWidth, int imageType, int id);

        ~ImageData();

        MessageType getMessageType() override;

        bool serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        [[nodiscard]] int getExpectedDataSize() const override;

        char *getDeserializeBuffer() override;

        bool deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        void setID(int id);

        virtual void setImage(unsigned char *_imageBytes, int _imageByteSize, int _imageHeight, int _imageWidth,
                              int _imageType);

        [[nodiscard]] int getID() const;

        [[nodiscard]] int getHeight() const;

        [[nodiscard]] int getWidth() const;

        [[nodiscard]] int getType() const;

        [[nodiscard]] char *getImageBytes() const;

        [[nodiscard]] int getImageBytesSize() const;

        [[nodiscard]] bool isImageDeserialized() const;

    protected:
        unsigned char *imageBytes;
        int id, imageHeight, imageWidth, imageType, contentSize;
        bool imageDeserialized;
    };
}

#endif //COMM_DATA_IMAGEDATA_H
