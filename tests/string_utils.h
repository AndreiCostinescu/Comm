//
// Created by Andrei Costinescu (andreicostinescu96@gmail.com) on 22.04.2021.
//

#ifndef COMMUNICATION_STRING_UTILS_H
#define COMMUNICATION_STRING_UTILS_H

#include <string>
#include <vector>

std::vector<std::string> splitString(const std::string &message, const std::string &splitter);

std::string trim(const std::string &str, const std::string &whitespace = " \t");

bool startsWith(const std::string &str, const std::string &startQuery);

bool endsWith(const std::string &str, const std::string &endQuery);

#endif //COMMUNICATION_STRING_UTILS_H
