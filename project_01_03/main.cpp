#include "../bigInt.h"
#include "../miller_rabin.h"
#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "program") << " <input_file> <output_file>" << std::endl;
        return 1;
    }

    std::string input_filename = argv[1];
    std::ifstream file(input_filename);
    if (!file) {
        std::cerr << "Cannot read file: " << input_filename << std::endl;
        return 1;
    }

    std::string output_filename = argv[2];
    std::ofstream outfile(output_filename);
    if (!outfile) {
        std::cerr << "Cannot open file for writing: " << output_filename << std::endl;
        return 1;
    }

    std::string n_str, k_str, x_str;
    file >> n_str >> k_str >> x_str;

    BigInt n(std::string(n_str.rbegin(), n_str.rend())); // Reverse the string
    BigInt k(std::string(k_str.rbegin(), k_str.rend())); // Reverse the string
    BigInt x(std::string(x_str.rbegin(), x_str.rend())); // Reverse the string
    file.close();

    std::string result = powMod(x, k, n).to_hex_string();
    result = std::string(result.rbegin(), result.rend()); // Reverse the string

    outfile << result << std::endl;
    outfile.close();

    return 0;
}