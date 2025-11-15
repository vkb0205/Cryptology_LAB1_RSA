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

    std::string testnum_str;
    file >> testnum_str;
    file.close();

    BigInt testnum(std::string(testnum_str.rbegin(), testnum_str.rend())); // Reverse the string
    
    int k = 40; // Number of rounds for Miller-Rabin
    bool result = is_prime_miller_rabin(testnum, k);

    outfile << result << std::endl;
    outfile.close();

    return 0;
}