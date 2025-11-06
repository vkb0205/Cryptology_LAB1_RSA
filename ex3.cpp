#include <iostream> // Dùng cho std::cin, std::cout, std::endl
#include <string>   // Dùng cho std::string
#include <sstream>  // Dùng cho std::stringstream
#include <iomanip>  // Dùng cho std::hex, std::uppercase, std::setfill, std::setw
#include "bigInt.h"
#include "miller_rabin.h"
#include <tuple>
#include <fstream>

int main() {
    BigInt k("B");
    BigInt x("1F");
    BigInt y("6AF");
    BigInt res = powMod_big(x, k, y);
    res.print_hex();
    return 0;
}