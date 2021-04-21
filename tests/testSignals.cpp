//
// Created by andrei on 09.04.21.
//

#include <comm/socket/utils.h>
#include <csignal>
#include <iostream>

using namespace comm;
using namespace std;

int main() {
    cout << "Hello World!" << endl;

    signal(SIGKILL, signalHandler);
    raise(SIGKILL);

    return 0;
}