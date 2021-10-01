//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 15-May-21.
//

#ifndef COMM_UTILS_NETWORKDATA_H
#define COMM_UTILS_NETWORKDATA_H

namespace comm {
    class NetworkData {
    public:
        static int bigEndian;

        static bool isBigEndian();

        static bool isLittleEndian();

        static void shortToNetworkBytes(char *buffer, int start, short value);

        static short networkBytesToShort(const char *buffer, int start);

        static void intToNetworkBytes(char *buffer, int start, int value);

        static int networkBytesToInt(const char *buffer, int start);

        static void longLongToNetworkBytes(char *buffer, int start, long long value);

        static long long networkBytesToLongLong(const char *buffer, int start);

        static void floatToNetworkBytes(char *buffer, int start, float value);

        static float networkBytesToFloat(const char *buffer, int start);

        static void doubleToNetworkBytes(char *buffer, int start, double value);

        static double networkBytesToDouble(const char *buffer, int start);
    };
}

#endif //COMM_UTILS_NETWORKDATA_H
