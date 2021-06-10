//
// Created by Andrei on 29-May-21.
//

#ifndef COMM_DATA_IMAGEENCODEDATA_H
#define COMM_DATA_IMAGEENCODEDATA_H

#include <comm/data/ImageData.h>
#include <vector>

namespace comm{
    class ImageEncodeData : public ImageData {
    public:
        static const int headerSize;

        enum Encoding {
            JPEG = 0,
            PNG,
            TIFF,
        };

        static std::string convertEncodingToString(Encoding encoding);

        static Encoding convertStringToEncoding(const std::string& encoding);

        ImageEncodeData();

        ImageEncodeData(cv::Mat image, int id, Encoding encoding);

        MessageType getMessageType() override;

        bool serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        [[nodiscard]] int getExpectedDataSize() const override;

        char *getDeserializeBuffer() override;

        bool deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        void setEncoding(Encoding _encoding);

        Encoding getEncoding();

    private:
        std::vector<uchar> encodedImage;
        int encodedContentSize;
        Encoding encoding;
    };
}

#endif //COMM_DATA_IMAGEENCODEDATA_H
