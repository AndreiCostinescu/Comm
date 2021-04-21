//
// Created by andrei on 21.04.21.
//

//
// Created by andrei on 30.11.20.
//

#include <comm/communication/Communication.h>
#include <comm/data/StatusData.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>

using namespace comm;
using namespace std;

 bool quit = false;

double angle(const cv::Point3f &p1, const cv::Point3f &p2) {
    double angle_cos = p1.dot(p2) / (cv::norm(p1) * cv::norm(p2));
    return acos(angle_cos);
}

bool calculateAngle(Buffer &buffer, bool verbose = false) {
    vector<double> angle_vector = {CV_PI/6, CV_PI/4, CV_PI/3, CV_PI/2, CV_PI/1, 2 * CV_PI};
    if (verbose) {
        char outputBuffer[54];
        sprintf(outputBuffer, "%08.3f %08.3f %08.3f %08.3f %08.3f %08.3f", angle_vector[0], angle_vector[1],
                angle_vector[2], angle_vector[3], angle_vector[4], angle_vector[5]);
        cout << outputBuffer << endl;
    }
    buffer.setBufferContentSize(6 * sizeof(double));
    for (int i = 0; i < angle_vector.size(); i++) {
        buffer.setDouble(angle_vector[i], (int) sizeof(double) * i);
    }
    return true;
}

void sendAngles(int argc, char **argv) {
    Communication comm;
    // comm.createSocket(SocketType::UDP, SocketPartner("131.159.203.138", 25001, false));
    // comm.createSocket(SocketType::UDP, SocketPartner("131.159.61.13", 25001, false));
    comm.createSocket(SocketType::UDP, SocketPartner("131.159.215.122", 25001, false));
    StatusData angleData;
    Buffer buffer(6 * sizeof(double));

    while (!quit) {
        if (calculateAngle(buffer, true)) {
            angleData.setData(buffer.getBuffer(), (int) buffer.getBufferContentSize());
            if (!comm.sendData(SocketType::UDP, &angleData, false)) {
                cout << "Could not send udp data... " << comm.getErrorString() << endl;
            }
            this_thread::sleep_for(chrono::milliseconds(500));
        }
    }
}

int main(int argc, char **argv) {
    cout << "Hello, World!" << endl;

    thread t(sendAngles, argc, argv);
    string x;
    while (true) {
        cin >> x;
        if (x == "q") {
            quit = true;
            break;
        }
    }
    t.join();
    return 0;
}
