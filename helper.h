// helper.h - declarations for big-integer helpers
#ifndef HELPER_H
#define HELPER_H

#include <string>

int cmpBig(const std::string &a, const std::string &b);
std::string addBig(const std::string &a, const std::string &b);
std::string subBig(std::string a, const std::string &b);
std::string div2Big(std::string a);
std::string modBig(const std::string &a, const std::string &b);
std::string mulBigMod(std::string a, std::string b, const std::string &mod);
std::string powBigMod(std::string a, std::string exp, const std::string &mod);

#endif // HELPER_H
