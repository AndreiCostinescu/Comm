//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 18-Jul-21.
//

#include <comm/communication/Communication.h>

using namespace comm;
using namespace std;

void tryBind(Communication &p, int localPort) {
    try {
        p.createSocket(SocketType::UDP, SocketPartner("127.0.0.1", 6666, false), localPort, 2000, 1);
        cout << "Bind succeeded" << endl;
    } catch (runtime_error &re) {
        if (strcmp(re.what(), "ERROR binding server socket") == 0) {
            cout << "Bind failed" << endl;
        } else {
            cout << "Unknown runtime error " << re.what() << endl;
        }
    }
}

int main() {
    Communication p1, p2;
    tryBind(p1, 6667);
    tryBind(p2, 6667);

    return 0;
}