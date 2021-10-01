//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 15-May-21.
//

#ifndef COMM_UTILS_SERIALIZATIONHEADER_H
#define COMM_UTILS_SERIALIZATIONHEADER_H

#include <comm/utils/Buffer.h>

namespace comm {
    class SerializationHeader {
    public:
        explicit SerializationHeader(bool create = true);

        explicit SerializationHeader(char *buffer);

        explicit SerializationHeader(Buffer &buffer);

        explicit SerializationHeader(int header);

        explicit SerializationHeader(unsigned char serializationIteration, unsigned char sendIteration,
                                     unsigned short sendSize, bool local = true);

        virtual ~SerializationHeader();

        void reset();

        void setInt(int header, bool local = true);

        void setData(unsigned char _serializationIteration, unsigned char _sendIteration, unsigned short _sendSize,
                     bool local = true);

        void setSerializationIteration(unsigned char _serializationIteration);

        void setSendIteration(unsigned char _sendIteration);

        void setSendSize(unsigned short _sendSize, bool setLocal = true);

        void setBuffer(char *buffer, bool setLocal, int start = 0) const;

        void setBuffer(Buffer &buffer, bool setLocal, int start = 0) const;

        void setSyphonUntilFirstMessage(bool _syphonUntilFirstMessage);

        [[nodiscard]] unsigned char getSerializationIteration() const;

        [[nodiscard]] unsigned char getSendIteration() const;

        [[nodiscard]] unsigned short getSendSize(bool getLocal = true) const;

        [[nodiscard]] int getInt(bool getLocal = true) const;

        [[nodiscard]] bool getSyphonUntilFirstMessage() const;

    private:
        char *localBuffer;
        bool created, syphonUntilFirstMessage;
        Buffer *passedBuffer;
    };
}

#endif //COMM_UTILS_SERIALIZATIONHEADER_H
