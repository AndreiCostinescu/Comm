//
// Created by ga78cat on 10.03.2021.
//

#include <comm/data/CoordinateData.h>
#include <comm/socket/utils.h>
#include <cassert>
#include <iostream>

using namespace comm;
using namespace std;

const int CoordinateData::headerSize = 2 * sizeof(int) + 2 * sizeof(double) + sizeof(long long);

CoordinateData::CoordinateData() : id(-1), time(0), x(0), y(0), touch(false), buttonFwd(false), buttonDwn(false) {}

MessageType CoordinateData::getMessageType() {
    return MessageType::COORDINATE;
}

bool CoordinateData::serialize(Buffer *buffer, int start, bool, bool verbose) {
    switch (this->serializeState) {
        case 0: {
            buffer->setBufferContentSize(CoordinateData::headerSize);
            if (verbose) {
                cout << "Serialize: " << this->id << ", " << this->time << ", " << this->x << ", " << this->y << ", "
                     << this->touch << ", " << this->buttonFwd << ", " << this->buttonDwn << ", " << endl;
            }
            buffer->setInt(this->id, start);
            buffer->setLongLong(this->time, start + 4);
            buffer->setDouble(this->x, start + 12);
            buffer->setDouble(this->y, start + 20);
            buffer->setInt((this->buttonDwn << 2) + (this->buttonFwd << 1) + this->touch, start + 28);
            this->serializeState = 0;
            return true;
        }
        default: {
            (*cerror) << "Impossible serialize state... " << this->serializeState << endl;
            this->serializeState = 0;
            return false;
        }
    }
}

int CoordinateData::getExpectedDataSize() const {
    switch (this->deserializeState) {
        case 0: {
            return CoordinateData::headerSize;
        }
        default : {
            throw runtime_error("Impossible deserialize state... " + to_string(this->deserializeState));
        }
    }
}

bool CoordinateData::deserialize(Buffer *buffer, int start, bool verbose) {
    switch (this->deserializeState) {
        case 0: {
            this->id = buffer->getInt(start);
            this->time = buffer->getLongLong(start + 4);
            this->x = buffer->getDouble(start + 12);
            this->y = buffer->getDouble(start + 20);
            int tmp = buffer->getInt(start + 28);
            this->touch = tmp & 1;
            this->buttonFwd = tmp & 2;
            this->buttonDwn = tmp & 4;
            this->deserializeState = 0;
            return true;
        }
        default: {
            (*cerror) << "Impossible deserialize state... " << this->deserializeState << endl;
            this->resetDeserializeState();
            return false;
        }
    }
}

void CoordinateData::set(const std::pair<std::string, void *> &value) {
    if (value.first == "id") {
        this->id = *(int *) value.second;
    } else if (value.first == "x") {
        this->x = *(double *) value.second;
    } else if (value.first == "y") {
        this->y = *(double *) value.second;
    } else if (value.first == "time") {
        this->time = *(long long *) value.second;
    } else if (value.first == "touch") {
        this->touch = *(bool *) value.second;
    } else if (value.first == "buttonFwd") {
        this->buttonFwd = *(bool *) value.second;
    } else if (value.first == "buttonDwd") {
        this->buttonDwn = *(bool *) value.second;
    }
}

void CoordinateData::set(const std::vector<std::pair<std::string, void *>> &values) {
    for (const auto &value : values) {
        this->set(value);
    }
}

int CoordinateData::getID() const {
    return this->id;
}

double CoordinateData::getX() const {
    return this->x;
}

double CoordinateData::getY() const {
    return this->y;
}

long long CoordinateData::getTime() const {
    return this->time;
}

bool CoordinateData::getTouch() const {
    return this->touch;
}
