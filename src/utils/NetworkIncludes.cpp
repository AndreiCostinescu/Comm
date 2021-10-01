//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 21.04.21.
//

#include <comm/utils/NetworkIncludes.h>

using namespace comm;
using namespace std;

#if defined(_WIN32) || defined(_WIN64)
bool comm::socketsStarted = false;
#else
bool comm::socketsStarted = true;
#endif

std::ostream *comm::cerror = &std::cout;