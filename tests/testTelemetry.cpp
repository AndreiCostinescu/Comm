//
// Created by Andrei on 09-Jun-21.
//

#include <chrono>
#include <comm/communication/Communication.h>
#include <comm/utils/Buffer.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include "map_utils.hpp"
#include "string_utils.h"

using namespace comm;
using namespace std;

class RobotTelemetryData {
public:
    explicit RobotTelemetryData(char id) : id(id), pose(7), force(3) {
        this->pose = {0.6877799401378936, 0.48034331143182685, 0.5749774323860651, -1.1119328873459091,
                      -1.3079182927409807, 1.4561646920568387, 1.6326282576521236};
    }

    void serialize(Buffer *buffer) const {
        buffer->setBufferContentSize(81);
        buffer->setChar(id, 0);
        int shift = 1;
        for (size_t i = 0; i < this->pose.size(); i++) {
            buffer->setDouble(this->pose[i], (int) (shift + i * 8));
        }
        shift += (int) (8 * this->pose.size());
        for (size_t i = 0; i < this->force.size(); i++) {
            buffer->setDouble(this->force[i], (int) (shift + i * 8));
        }
    }

    void setPose(int position, double value) {
        this->pose[position] = value;
    }

    void incrementPose(int position, double increment) {
        this->pose[position] += increment;
    }

    void setForce(int position, double value) {
        this->force[position] = value;
        cout << this->force[0] << ", " << this->force[1] << ", " << this->force[2] << endl;
    }

    void incrementForce(int position, double increment) {
        this->force[position] += increment;
    }

    char id;
    vector<double> pose;
    vector<double> force;
};

bool quitFlag = false;
mutex robotDataAccessLock;

void receiveDataThread() {
    Communication digitalTwinReceiver;
    digitalTwinReceiver.createSocket(SocketType::UDP, nullptr, 8400, -1, 5);  // recv timeout 5ms
    Buffer telemetryBuffer;
    bool receivedData = false;
    string data;
    int dataShift;
    telemetryBuffer.setBufferContentSize(81);
    while (!quitFlag) {
        if (!digitalTwinReceiver.receiveRaw(SocketType::UDP, telemetryBuffer.getBufferReference(), receivedData)) {
            cout << "Error when receiving data..." << endl;
            quitFlag = true;
        }
        if (receivedData) {
            data = "Received data from robot " + to_string((int) telemetryBuffer.getChar(0)) + "\n";
            data += "Pose: ";
            dataShift = 1;
            for (int i = 0; i < 7; i++) {
                if (i != 0) {
                    data += ", ";
                }
                data += to_string(telemetryBuffer.getDouble(dataShift + i * 8));
            }
            data += "\nForce: ";
            dataShift += 56;  // 7 * 8
            for (int i = 0; i < 3; i++) {
                if (i != 0) {
                    data += ", ";
                }
                data += to_string(telemetryBuffer.getDouble(dataShift + i * 8));
            }
            data += "\n\n";
            cout << data;
        }
    }
}

void sendDataThread(const string &ip, int port, map<int, RobotTelemetryData *> *allRobotData) {
    if (allRobotData == nullptr) {
        return;
    }

    Communication digitalTwinMaster;
    digitalTwinMaster.createSocket(SocketType::UDP, SocketPartner(ip, port, false));
    Buffer telemetryData;

    while (!quitFlag) {
        robotDataAccessLock.lock();
        cout << "Sending data..." << endl;
        for (const auto &robotData : *allRobotData) {
            robotData.second->serialize(&telemetryData);
            digitalTwinMaster.sendRaw(SocketType::UDP, telemetryData.getConstBuffer(),
                                      telemetryData.getBufferContentSize());
        }
        robotDataAccessLock.unlock();
        this_thread::sleep_for(chrono::seconds(5));
    }

}

int main(int argc, char **argv) {
    string ip = "127.0.0.1";
    int port = 8400;
    double poseIncrement = 0.1, forceIncrement = 0.1;
    for (int i = 1; i < argc; i++) {
        if (i == 1) {
            ip = argv[i];
        } else if (i == 2) {
            port = stoi(argv[i]);
        } else if (i == 3) {
            poseIncrement = stod(argv[i]);
            forceIncrement = poseIncrement;
        } else if (i == 4) {
            forceIncrement = stod(argv[i]);
        }
    }

    map<int, RobotTelemetryData *> allRobotData;
    thread recvThread = thread(receiveDataThread);
    thread sendThread = thread(sendDataThread, ip, port, &allRobotData);

    string x;
    vector<string> inputSplit;
    RobotTelemetryData *currentRobotData = nullptr;
    int robotID = -1;
    int poseIndex = 0, forceIndex = 0;
    double poseValue, forceValue;
    int lastOperation = 0;
    try {
        while (!quitFlag) {
            getline(cin, x);
            if (startsWith(x, "id ")) {
                inputSplit = splitString(x, " ");
                if (inputSplit.size() != 2) {
                    cout << "Format: \"id <type id>\"" << endl;
                    continue;
                }
                int id = stoi(inputSplit[1]);
                if (id < 0 || id > 255) {
                    cout << "ID between 0 and 255 inclusive" << endl;
                    continue;
                }
                robotID = id;
                if (!mapContains(allRobotData, id, &currentRobotData)) {
                    robotDataAccessLock.lock();
                    currentRobotData = new RobotTelemetryData((char) id);
                    allRobotData[id] = currentRobotData;
                    robotDataAccessLock.unlock();
                }
            } else if (startsWith(x, "p ") || x == "p") {
                inputSplit = splitString(x, " ");
                if (inputSplit.size() >= 2) {
                    if (stoi(inputSplit[1]) >= 0 && stoi(inputSplit[1]) <= 6) {
                        poseIndex = stoi(inputSplit[1]);
                    }
                }
                if (inputSplit.size() >= 3) {
                    poseValue = stod(inputSplit[2]);
                    if (currentRobotData != nullptr) {
                        robotDataAccessLock.lock();
                        currentRobotData->setPose(poseIndex, poseValue);
                        robotDataAccessLock.unlock();
                    }
                }

                lastOperation = 1;
            } else if (startsWith(x, "f ") || x == "f") {
                inputSplit = splitString(x, " ");
                if (inputSplit.size() >= 2) {
                    if (stoi(inputSplit[1]) >= 0 && stoi(inputSplit[1]) <= 2) {
                        forceIndex = stoi(inputSplit[1]);
                    }
                }
                if (inputSplit.size() >= 3) {
                    forceValue = stod(inputSplit[2]);
                    if (currentRobotData != nullptr) {
                        robotDataAccessLock.lock();
                        currentRobotData->setForce(forceIndex, forceValue);
                        robotDataAccessLock.unlock();
                    }
                }

                lastOperation = 2;
            } else if (x == "a" || x == "s") {
                if (currentRobotData == nullptr) {
                    continue;
                }
                if (lastOperation == 1) {
                    robotDataAccessLock.lock();
                    currentRobotData->incrementPose(poseIndex, -poseIncrement);
                    robotDataAccessLock.unlock();
                } else if (lastOperation == 2) {
                    robotDataAccessLock.lock();
                    currentRobotData->incrementForce(forceIndex, -forceIncrement);
                    robotDataAccessLock.unlock();
                }
            } else if (x == "d" || x == "w") {
                if (currentRobotData == nullptr) {
                    continue;
                }
                if (lastOperation == 1) {
                    robotDataAccessLock.lock();
                    currentRobotData->incrementPose(poseIndex, poseIncrement);
                    robotDataAccessLock.unlock();
                } else if (lastOperation == 2) {
                    robotDataAccessLock.lock();
                    currentRobotData->incrementForce(forceIndex, forceIncrement);
                    robotDataAccessLock.unlock();
                }
            } else if (x == "q") {
                quitFlag = true;
                break;
            }
            if (currentRobotData != nullptr) {
                cout << "At robot id " << robotID << "\npose index " << poseIndex << "\nforce index " << forceIndex
                     << endl;
            }
        }
    } catch (exception &e) {
        cout << "Caught exception: " << e.what();
        quitFlag = true;
    }

    quitFlag = true;
    recvThread.join();
    sendThread.join();

    for (const auto &robotData : allRobotData) {
        delete robotData.second;
    }
    allRobotData.clear();

    return 0;
}
