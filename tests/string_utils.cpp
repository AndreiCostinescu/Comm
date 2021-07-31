//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 22.04.2021.
//

#include "string_utils.h"
#include <algorithm>
#include <cassert>

using namespace std;

vector<string> splitString(const string &message, const string &splitter) {
    // cout << "Message length: " << message.size() << endl;
    vector<string> res;
    size_t start = 0;
    size_t end = message.find(splitter);
    while (start < message.size() && start != string::npos) {
        auto split = message.substr(start, end - start);
        // cout << "\"" << split << "\"; start = " << start << "; end = " << end << endl;
        res.push_back(split);
        start = end + ((string::npos - end >= splitter.size()) ? splitter.size() : 0);
        end = message.find(splitter, start);
    }
    return res;
}

string trim(const string &str, const string &whitespace) {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == string::npos) {
        return "";
    } // no content

    const auto strEnd = str.find_last_not_of(whitespace);

    return str.substr(strBegin, strEnd - strBegin + 1);
}

bool startsWith(const string &str, const string &startQuery) {
    return (str.rfind(startQuery, 0) == 0);
}

bool endsWith(const string &str, const string &endQuery) {
    if (str.length() >= endQuery.length()) {
        return (0 == str.compare(str.length() - endQuery.length(), endQuery.length(), endQuery));
    }
    return false;
}
