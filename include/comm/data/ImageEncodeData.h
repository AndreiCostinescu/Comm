//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 29-May-21.
//

#ifndef COMM_DATA_IMAGEENCODEDATA_H
#define COMM_DATA_IMAGEENCODEDATA_H

#include <comm/data/ImageData.h>
#include <vector>

namespace comm {
    class ImageEncodeData : public virtual ImageData {
    public:
        static const int headerSize;

        enum Encoding {
            JPEG = 0,
            PNG,
            TIFF,
        };

        static std::string convertEncodingToString(Encoding encoding);

        static Encoding convertStringToEncoding(const std::string &encoding);

        ImageEncodeData();

        ImageEncodeData(const std::vector<unsigned char> &imageEncodedBytes, int imageHeight, int imageWidth,
                        int imageType, int id, Encoding encoding);

        ~ImageEncodeData() override;

        MessageType getMessageType() override;

        bool serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        [[nodiscard]] int getExpectedDataSize() const override;

        char *getDeserializeBuffer() override;

        bool deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        void setEncoding(Encoding _encoding);

        Encoding getEncoding();

        void setImageEncodedBytes(const std::vector<unsigned char> &imageEncodedBytes, int imageHeight, int imageWidth,
                                  int imageType, int id, Encoding _encoding);

    protected:
        std::vector<unsigned char> encodedImage;
        int encodedContentSize;
        Encoding encoding;
    };
}

#endif //COMM_DATA_IMAGEENCODEDATA_H
