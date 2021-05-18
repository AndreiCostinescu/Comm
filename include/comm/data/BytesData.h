//
// Created by Andrei on 25-Apr-21.
//

#ifndef COMMUNICATION_BYTESDATA_H
#define COMMUNICATION_BYTESDATA_H

#include <comm/data/CommunicationData.h>
#include <comm/utils/Buffer.h>

namespace comm {
    class BytesData : public CommunicationData {
    public:
        static const int headerSize;

        explicit BytesData(int size = 0);

        ~BytesData() override;

        MessageType getMessageType() override;

        bool serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        [[nodiscard]] int getExpectedDataSize() const override;

        bool deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        void reset();

        void setData(char *_data, unsigned int dataSize, int start = 0);

        void setData(const char *_data, unsigned int dataSize, int start = 0);

        void setReferenceToData(char *_data, unsigned int dataSize);

        void setChar(char _data, int position);

        void setShort(short _data, int position);

        void setInt(int _data, int position);

        void setLongLong(long long _data, int position);

        void setFloat(float _data, int position);

        void setDouble(double _data, int position);

        char getChar(int position);

        short getShort(int position);

        int getInt(int position);

        long long getLongLong(int position);

        float getFloat(int position);

        double getDouble(int position);

        [[nodiscard]] bool empty() const;

        [[nodiscard]] const char *getBuffer();

        [[nodiscard]] uint64_t getBufferSize() const;

    private:
        Buffer data;
        int expectedDataSize;
    };


}

#endif //COMMUNICATION_BYTESDATA_H
