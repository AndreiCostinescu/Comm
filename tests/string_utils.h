//
// Created by ga78cat on 22.04.2021.
//

#ifndef COMMUNICATION_STRING_UTILS_H
#define COMMUNICATION_STRING_UTILS_H

#include <string>
#include <vector>

std::vector<std::string> splitString(const std::string &message, const std::string &splitter);

std::string trim(const std::string &str, const std::string &whitespace = " \t");

#endif //COMMUNICATION_STRING_UTILS_H
