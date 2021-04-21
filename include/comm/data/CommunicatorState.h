//
// Created by ga78cat on 24.03.2021.
//

#ifndef PINAKOTHEKDRAWING_COMMUNICATORSTATE_H
#define PINAKOTHEKDRAWING_COMMUNICATORSTATE_H

#include <string>

namespace comm {
    enum CommunicatorState {
        COMMUNICATOR_IDLE = 0,
        COMMUNICATOR_ACTIVE = 1,
        COMMUNICATOR_DONE = 2,
    };

    CommunicatorState stringToCommunicatorState(const std::string &s);

    std::string communicatorStateToString(const CommunicatorState &communicatorState);

    std::string convertCommunicatorStateToStatus(const CommunicatorState &communicatorState);
}

#endif //PINAKOTHEKDRAWING_COMMUNICATORSTATE_H
