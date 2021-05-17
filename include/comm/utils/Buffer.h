//
// Created by ga78cat on 24.03.2021.
//

#ifndef PINAKOTHEKDRAWING_BUFFER_H
#define PINAKOTHEKDRAWING_BUFFER_H

namespace comm {
    class Buffer {
    public:
        explicit Buffer(unsigned long long int bufferSize = 0);

        ~Buffer();

        void reset();

        [[nodiscard]] Buffer *copy() const;

        void setData(char *data, unsigned int dataSize, int start = 0);

        void setData(const char *data, unsigned int dataSize, int start = 0);

        void setReferenceToData(char *data, unsigned int dataSize);

        void setChar(char data, int position);

        void setShort(short data, int position);

        void setInt(int data, int position);

        void setLongLong(long long data, int position);

        void setFloat(float data, int position);

        void setDouble(double data, int position);

        char getChar(int position);

        short getShort(int position);

        int getInt(int position);

        long long getLongLong(int position);

        float getFloat(int position);

        double getDouble(int position);

        [[nodiscard]] bool empty() const;

        [[nodiscard]] char *getBuffer();

        [[nodiscard]] unsigned long long int getBufferSize() const;

        [[nodiscard]] unsigned long long int getBufferContentSize() const;

        void setBufferContentSize(unsigned long long int _bufferContentSize);

    private:
        void prepareBuffer(unsigned long long int desiredSize);

        void checkBufferContentSize(unsigned long long int size, bool modifySize);

        char *buffer, *referenceBuffer;
        unsigned long long int bufferSize, bufferContentSize;
        bool useReferenceBuffer;
    };
}


#endif //PINAKOTHEKDRAWING_BUFFER_H
