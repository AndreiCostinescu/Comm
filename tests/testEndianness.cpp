//
// Created by ga78cat on 17.03.2021.
//

#include <cassert>
#include <comm/socket/utils.h>
#include <iostream>

using namespace comm;
using namespace std;

int main() {
    cout << "Hello World!" << endl;
    cout << "Am I big endian? " << isBigEndian() << endl;
    cout << sizeof(char) << " " << sizeof(short) << " " << sizeof(int) << " " << sizeof(long) << " "
         << sizeof(long long) << endl;

    u_short t = 48585;
    cout << t << ", " << ntohs(t) << ", " << htons(t) << endl;

    char *buffer = nullptr;
    int bufferSize = 0;

    long long x = 1ll << 63, resLL;
    for (int factor = 2; factor <= 15; factor++) {
        cout << "Factor = " << factor << endl;
        cout << "Factor = " << factor << endl;
        long long prevY = -1;
        for (long long y = 0; y < ((1ll << 63) - 1) && y >= 0; y++, y *= factor) {
            if (prevY == y) {
                break;
            }
            cout << "y = " << y << endl;
            prepareBuffer(buffer, bufferSize, sizeof(long long));
            longLongToNetworkBytes(buffer, 0, y);
            resLL = networkBytesToLongLong(buffer, 0);
            if (resLL != y) {
                cout << "Error at y = " << y << ", res = " << resLL << endl;
            }
            assert(resLL == y);
            prevY = y;
        }
    }

    cout << x << endl;
    prepareBuffer(buffer, bufferSize, sizeof(long long));
    longLongToNetworkBytes(buffer, 0, x);
    assert(networkBytesToLongLong(buffer, 0) == x);

    int res = 0;
    for (int y = 0; y < 1 << 20; y++) {
        prepareBuffer(buffer, bufferSize, sizeof(int));
        intToNetworkBytes(buffer, 0, y);
        res = networkBytesToInt(buffer, 0);
        if (res != y) {
            cout << "Error at y = " << y << ", res = " << res << endl;
        }
        assert(res == y);
    }
    int yI = 1 << 31;
    cout << yI << endl;
    prepareBuffer(buffer, bufferSize, sizeof(int));
    intToNetworkBytes(buffer, 0, yI);
    assert(networkBytesToInt(buffer, 0) == yI);

    short resS = 0;
    for (short y = 0; y < 1 << 13; y++) {
        prepareBuffer(buffer, bufferSize, sizeof(short));
        shortToNetworkBytes(buffer, 0, y);
        resS = networkBytesToShort(buffer, 0);
        if (resS != y) {
            cout << "Error at y = " << y << ", res = " << resS << endl;
        }
        assert(resS == y);
    }

    double resF;
    for (int factor = 2; factor <= 5; factor++) {
        for (float z = 1.0; z > 0; z /= factor) {
            cout << z << endl;
            prepareBuffer(buffer, bufferSize, sizeof(float));
            floatToNetworkBytes(buffer, 0, z);
            resF = networkBytesToFloat(buffer, 0);
            if (resF != z) {
                cout << "Error at z = " << z << ", res = " << resF << endl;
            }
            assert(resF == z);
        }
    }

    double resD;
    for (int factor = 2; factor <= 5; factor++) {
        for (double z = 1.0; z > 0; z /= factor) {
            cout << z << endl;
            prepareBuffer(buffer, bufferSize, sizeof(double));
            doubleToNetworkBytes(buffer, 0, z);
            resD = networkBytesToDouble(buffer, 0);
            if (resD != z) {
                cout << "Error at z = " << z << ", res = " << resD << endl;
            }
            assert(resD == z);
        }
    }

}