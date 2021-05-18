//
// Created by ga78cat on 10.03.2021.
//

#ifndef PINAKOTHEKDRAWING_COORDINATEDATA_H
#define PINAKOTHEKDRAWING_COORDINATEDATA_H

#include <comm/data/CommunicationData.h>
#include <string>
#include <vector>

namespace comm {
    class CoordinateData : public CommunicationData {
    public:
        const static int headerSize;

        CoordinateData();

        MessageType getMessageType() override;

        bool serialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        [[nodiscard]] int getExpectedDataSize() const override;

        bool deserialize(Buffer *buffer, int start, bool forceCopy, bool verbose) override;

        void set(const std::pair<std::string, void *> &value);

        void set(const std::vector<std::pair<std::string, void *>> &values);

        [[nodiscard]] int getID() const;

        [[nodiscard]] double getX() const;

        [[nodiscard]] double getY() const;

        [[nodiscard]] long long getTime() const;

        [[nodiscard]] bool getTouch() const;

    private:
        int id;
        double x, y;
        long long time;
        bool touch, buttonFwd, buttonDwn;
    };
}

#endif //PINAKOTHEKDRAWING_COORDINATEDATA_H
