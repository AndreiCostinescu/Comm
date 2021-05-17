//
// Created by ga78cat on 23.04.2021.
//

#include <comm/communication/Communication.h>
#include <comm/data/ImageData.h>
#include <comm/data/StatusData.h>
#include <comm/socket/utils.h>
#include <iostream>
#include <thread>

using namespace comm;
using namespace cv;
using namespace std;

bool udpStopFlag = true;
bool tcpStopFlag = true;
int limit = -1;

void udpStreamer(SocketType socketType) {
    Communication p;
    p.createSocket(socketType, SocketPartner("127.0.0.1", 8400, false), 0, 2000, 500);
    cout << "Initialized sender udp server!" << endl;

    cv::VideoCapture camera(0);
    cv::namedWindow("Camera image");
    // Mat image = cv::imread("../../data/Lena.png");
    Mat image;
    ImageData i;
    int id = 0;
    while (!udpStopFlag && (limit < 0 or id < limit)) {
        camera >> image;
        i.setImage(image);
        cout << "image size " << image.size() << " and id " << id << endl;
        i.setID(id);

        if (!p.transmitData(socketType, &i, false, true, 0, false)) {
            cout << "Error: " << p.getErrorCode() << "; " << p.getErrorString() << "; " << getLastErrorString() << endl;
            break;
        } else {
            cout << "Sent image data " << endl;
        }

        cv::imshow("Camera image", image);
        int k = cv::waitKey(20);
        if (k == 'q' || k == 27) {
            break;
        }
        id++;
    }

    camera.release();
    cout << "Streamer finished normally" << endl;
}

void tcpStreamer() {
    SocketType socketType = SocketType::TCP;
    Communication p;
    try {
        p.createSocket(socketType, SocketPartner("127.0.0.1", 8400, false), 0, 2000, 500);
    } catch (runtime_error &re) {
        auto errorMessage = re.what();
        if (strcmp(errorMessage, "Connection failed due to port and ip problems!") == 0) {
            return;
        }
    }
    cout << "Initialized sender tcp server!" << endl;

    cv::VideoCapture camera(0);
    camera.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cv::namedWindow("Camera image");
    // Mat image = cv::imread("../../data/Lena.png");
    Mat image;
    ImageData i;
    int id = 0;
    while (!tcpStopFlag && (limit < 0 or id < limit)) {
        camera >> image;
        i.setImage(image);
        cout << "image size " << image.size() << " and id " << id << endl;
        i.setID(id);

        if (!p.transmitData(socketType, &i, false, true, 0, false)) {
            cout << "Error: " << p.getErrorCode() << "; " << p.getErrorString() << "; " << getLastErrorString() << endl;
            break;
        } else {
            cout << "Sent image data " << endl;
        }

        cv::imshow("Camera image", image);
        int k = cv::waitKey(2);
        if (k == 'q' || k == 27) {
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(25));
        id++;
    }

    StatusData s("quit");
    p.transmitData(socketType, &s, false, true);

    camera.release();
    cv::destroyAllWindows();
    cout << "Streamer finished normally" << endl;
}

int main() {
    cout << "Hello World!" << endl;

    string x;
    thread t, u;
    while (true) {
        cin >> x;
        if (x == "q") {
            if (!udpStopFlag) {
                udpStopFlag = true;
                u.join();
            }
            if (!tcpStopFlag) {
                tcpStopFlag = true;
                t.join();
            }
            break;
        } else if (x == "u") {
            if (udpStopFlag) {
                udpStopFlag = false;
                u = thread(udpStreamer, SocketType::UDP_HEADER);
            } else {
                udpStopFlag = true;
                u.join();
            }
        } else if (x == "t") {
            if (tcpStopFlag) {
                tcpStopFlag = false;
                t = thread(tcpStreamer);
            } else {
                tcpStopFlag = true;
                t.join();
            }
        }
    }

    return 0;
}