//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 24.03.2021.
//

#include <comm/data/CommunicatorState.h>
#include <comm/data/messages.h>
#include <stdexcept>

using namespace comm;
using namespace std;

CommunicatorState comm::stringToCommunicatorState(const string &s) {
    if (s == "idle") {
        return CommunicatorState::COMMUNICATOR_IDLE;
    } else if (s == "active") {
        return CommunicatorState::COMMUNICATOR_ACTIVE;
    } else if (s == "done") {
        return CommunicatorState::COMMUNICATOR_DONE;
    }
    throw runtime_error("Can not convert " + s + " to CommunicatorState enum");
}

string comm::communicatorStateToString(const CommunicatorState &communicatorState) {
    switch (communicatorState) {
        case CommunicatorState::COMMUNICATOR_IDLE: {
            return "idle";
        }
        case CommunicatorState::COMMUNICATOR_ACTIVE: {
            return "active";
        }
        case CommunicatorState::COMMUNICATOR_DONE: {
            return "done";
        }
        default: {
            throw runtime_error("Undefined CommunicatorState: " + to_string(int(communicatorState)));
        }
    }
}

const char *comm::convertCommunicatorStateToStatus(const CommunicatorState &communicatorState) {
    switch (communicatorState) {
        case CommunicatorState::COMMUNICATOR_IDLE: {
            return IDLE_MESSAGE;
        }
        case CommunicatorState::COMMUNICATOR_ACTIVE: {
            return ACTIVE_MESSAGE;
        }
        case CommunicatorState::COMMUNICATOR_DONE: {
            return DONE_MESSAGE;
        }
        default: {
            throw runtime_error("Unknown CommunicatorState " + to_string(int(communicatorState)));
        }
    }
}
