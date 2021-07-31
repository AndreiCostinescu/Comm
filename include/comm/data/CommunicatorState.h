//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 24.03.2021.
//

#ifndef COMM_DATA_COMMUNICATORSTATE_H
#define COMM_DATA_COMMUNICATORSTATE_H

#include <string>

namespace comm {
    enum CommunicatorState {
        COMMUNICATOR_IDLE = 0,
        COMMUNICATOR_ACTIVE = 1,
        COMMUNICATOR_DONE = 2,
    };

    CommunicatorState stringToCommunicatorState(const std::string &s);

    std::string communicatorStateToString(const CommunicatorState &communicatorState);

    const char *convertCommunicatorStateToStatus(const CommunicatorState &communicatorState);
}

#endif //COMM_DATA_COMMUNICATORSTATE_H
