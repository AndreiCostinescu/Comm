//
// Created by Andrei on 15-May-21.
//

#ifndef COMMUNICATION_SERIALIZATIONHEADER_H
#define COMMUNICATION_SERIALIZATIONHEADER_H

#include <comm/utils/Buffer.h>

namespace comm {
    class SerializationHeader {
    public:
        SerializationHeader();

        explicit SerializationHeader(int header);

        explicit SerializationHeader(unsigned char serializationIteration, unsigned char sendIteration,
                                     unsigned short sendSize);

        void reset();

        void fromInt(int header, bool local = true);

        void fromData(unsigned char _serializationIteration, unsigned char _sendIteration, unsigned short _sendSize,
                      bool local = true);

        void setBuffer(char *buffer, bool setLocal, int start = 0) const;

        void setBuffer(Buffer buffer, bool setLocal, int start = 0) const;

        [[nodiscard]] unsigned char getSerializationIteration() const;

        [[nodiscard]] unsigned char getSendIteration() const;

        [[nodiscard]] unsigned short getSendSize(bool getLocal = true) const;

        [[nodiscard]] int getInt(bool getLocal = true) const;

    private:
        unsigned char serializationIteration;
        unsigned char sendIteration;
        unsigned short sendSize;
    };
}

#endif //COMMUNICATION_SERIALIZATIONHEADER_H
