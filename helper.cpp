#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>      // For uint64_t, uint32_t
#include <algorithm>    // For std::reverse, std::max
#include <stdexcept>    // For std::runtime_error
#include <random>       // For Miller-Rabin
#include <iomanip>      // For std::setw, std::setfill

// --- FOR 128-BIT ARITHMETIC ---
struct uint128_t {
    uint64_t lo, hi;
    uint128_t(uint64_t l = 0, uint64_t h = 0) : lo(l), hi(h) {}
    // Simplified constructor for this problem
    uint128_t(uint64_t l) : lo(l), hi(0) {}
};


/**
 * @brief Layer 1: The BigInt Class
 * Stores a large unsigned integer as a vector of 64-bit "limbs".
 * Limbs are little-endian (limbs[0] is the least significant).
 */
class BigInt {
public:
    std::vector<uint64_t> limbs;

    // --- Constructors ---
    BigInt() { limbs.push_back(0); } // Default constructor (value 0)
    BigInt(uint64_t n) { limbs.push_back(n); normalize(); } // From int

    // Constructor from a big-endian hex string
    BigInt(const std::string& hex_str) {
        if (hex_str.empty()) {
            limbs.push_back(0);
            return;
        }
        // Each hex char is 4 bits. 16 hex chars = 64 bits (one limb).
        int len = hex_str.length();
        int num_limbs = (len + 15) / 16;
        limbs.resize(num_limbs, 0);

        int limb_idx = 0;
        int count = 0;
        uint64_t current_limb = 0;

        for (int i = len - 1; i >= 0; --i) {
            char c = hex_str[i];
            uint64_t val;
            if (c >= '0' && c <= '9') val = c - '0';
            else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
            else throw std::runtime_error("Invalid hex character");

            current_limb |= (val << (count * 4));
            count++;
            if (count == 16) {
                limbs[limb_idx++] = current_limb;
                current_limb = 0;
                count = 0;
            }
        }
        if (count > 0) {
            limbs[limb_idx] = current_limb;
        }
        normalize();
    }

    // --- Helper Functions ---
    void normalize() {
        while (limbs.size() > 1 && limbs.back() == 0) {
            limbs.pop_back();
        }
    }
    
    bool is_zero() const {
        return limbs.size() == 1 && limbs[0] == 0;
    }

    bool is_even() const {
        return (limbs[0] & 1) == 0;
    }

    // --- Comparison Operators ---
    friend bool operator==(const BigInt& a, const BigInt& b) {
        return a.limbs == b.limbs;
    }
    friend bool operator!=(const BigInt& a, const BigInt& b) {
        return a.limbs != b.limbs;
    }
    friend bool operator<(const BigInt& a, const BigInt& b) {
        if (a.limbs.size() != b.limbs.size()) {
            return a.limbs.size() < b.limbs.size();
        }
        for (int i = a.limbs.size() - 1; i >= 0; --i) {
            if (a.limbs[i] != b.limbs[i]) {
                return a.limbs[i] < b.limbs[i];
            }
        }
        return false; // They are equal
    }
    friend bool operator>(const BigInt& a, const BigInt& b) { return b < a; }
    friend bool operator<=(const BigInt& a, const BigInt& b) { return !(a > b); }
    friend bool operator>=(const BigInt& a, const BigInt& b) { return !(a < b); }

    // --- Bitwise Shift Operators (Needed for Division) ---
    friend BigInt operator<<(const BigInt& a, size_t shift_bits) {
        BigInt result = a;
        size_t shift_limbs = shift_bits / 64;
        size_t inner_shift = shift_bits % 64;

        if (inner_shift > 0) {
            uint64_t carry = 0;
            for (size_t i = 0; i < result.limbs.size(); ++i) {
                uint64_t next_carry = result.limbs[i] >> (64 - inner_shift);
                result.limbs[i] = (result.limbs[i] << inner_shift) | carry;
                carry = next_carry;
            }
            if (carry > 0) result.limbs.push_back(carry);
        }
        if (shift_limbs > 0) {
            result.limbs.insert(result.limbs.begin(), shift_limbs, 0);
        }
        result.normalize();
        return result;
    }
    friend BigInt operator>>(const BigInt& a, size_t shift_bits) {
        BigInt result = a;
        size_t shift_limbs = shift_bits / 64;
        size_t inner_shift = shift_bits % 64;

        if (shift_limbs >= result.limbs.size()) return BigInt(0);
        if (shift_limbs > 0) {
            result.limbs.erase(result.limbs.begin(), result.limbs.begin() + shift_limbs);
        }
        if (inner_shift > 0) {
            uint64_t borrow = 0;
            for (int i = result.limbs.size() - 1; i >= 0; --i) {
                uint64_t next_borrow = result.limbs[i] << (64 - inner_shift);
                result.limbs[i] = (result.limbs[i] >> inner_shift) | borrow;
                borrow = next_borrow;
            }
        }
        result.normalize();
        return result;
    }

    // --- Layer 2: Core Arithmetic ---

    // // Addition (a + b)
    // friend BigInt operator+(const BigInt& a, const BigInt& b) {
        
    // }

    // // Subtraction (a - b)
    // friend BigInt operator-(const BigInt& a, const BigInt& b) {
        
    // }

    // // Multiplication (a * b) - Grade School O(n^2)
    // friend BigInt operator*(const BigInt& a, const BigInt& b) {
        
    // }
    
    // // Division and Modulo (a / b) and (a % b)
    // // Returns {quotient, remainder}
    // static std::pair<BigInt, BigInt> divmod(const BigInt& dividend_in, const BigInt& divisor_in) {
        
    // }

    // friend BigInt operator/(const BigInt& a, const BigInt& b) {
    //     return divmod(a, b).first;
    // }
    // friend BigInt operator%(const BigInt& a, const BigInt& b) {
    //     return divmod(a, b).second;
    // }

    /**
     * @brief Prints the BigInt to std::cout as a base-10 decimal string.
     * This is slow for very large numbers due to repeated division.
     */
    // void print_decimal() const {
    //     // Handle the zero case
    //     if (is_zero()) {
    //         std::cout << "0" << std::endl;
    //         return;
    //     }

    //     BigInt temp = *this;
    //     std::string decimal_str;
    //     const BigInt TEN(10); // Create a BigInt for 10

    //     while (!temp.is_zero()) {
    //         // Use the divmod we already built
    //         std::pair<BigInt, BigInt> result = divmod(temp, TEN);
            
    //         BigInt& quotient = result.first;
    //         BigInt& remainder = result.second; // This will be 0-9

    //         // The remainder's value is in its first (and only) limb
    //         uint64_t digit = remainder.limbs[0];
    //         decimal_str += (char)(digit + '0');

    //         temp = quotient; // Update temp = temp / 10
    //     }

    //     // The digits were added in reverse order (LSD first)
    //     std::reverse(decimal_str.begin(), decimal_str.end());

    //     std::cout << decimal_str << std::endl;
    // }

    /**
     * @brief Prints the BigInt to std::cout as a base-16 hexadecimal string.
     * This is very fast as it maps directly to the limb storage.
     */
    void print_hex() const {
        // Handle the zero case
        if (is_zero()) {
            std::cout << "0x0" << std::endl;
            return;
        }

        // Set up cout for hex output
        std::cout << "0x" << std::hex;

        // Print the most significant limb first, without padding
        std::cout << limbs.back();

        // Print the rest of the limbs, padding with '0' to 16 chars
        for (int i = limbs.size() - 2; i >= 0; --i) {
            std::cout << std::setw(16) << std::setfill('0') << limbs[i];
        }

        // Reset cout to decimal for future use
        std::cout << std::dec << std::endl;
        for (int i = 0; i < limbs.size(); i++) {
            std::cout << limbs[i] << std::endl;
        }
    }
};

// --- Layer 3: The Algorithms ---

/**
 * @brief Modular Exponentiation: (base ^ exp) % mod
 * Uses exponentiation by squaring.
 */
// BigInt modular_pow(BigInt base, BigInt exp, const BigInt& mod) {
//     BigInt result(1);
//     base = base % mod;
//     while (!exp.is_zero()) {
//         // If exponent is odd, multiply result by base
//         if (!exp.is_even()) {
//             result = (result * base) % mod;
//         }
//         // Exponent is now even, divide by 2
//         exp = exp >> 1;
//         // Square the base
//         base = (base * base) % mod;
//     }
//     return result;
// }

/**
 * @brief Parses a little-endian hex string into a big-endian hex string.
 * "A1B2C3D4" (bytes A1, B2, C3, D4) -> "D4C3B2A1"
 */
std::string parse_little_endian_hex(const std::string& le_hex) {
    if (le_hex.length() % 2 != 0) {
        throw std::runtime_error("Input hex string has odd length.");
    }
    std::string be_hex;
    // Reserve space for the new string
    be_hex.reserve(le_hex.length());
    
    // Iterate from the end, taking 2-char (byte) chunks
    for (int i = le_hex.length() - 2; i >= 0; i -= 2) {
        be_hex.push_back(le_hex[i]);
        be_hex.push_back(le_hex[i + 1]);
    }
    return be_hex;
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    std::string le_hex_str;
    std::getline(file, le_hex_str);
    file.close();
    return le_hex_str;
}


// --- Main Program ---
int main() {
    for(int i = 0; i < 20; i++){
        std::string tmp;
        if(i < 10) {
            tmp = "test\\project_01_01\\test_0" + std::to_string(i) + ".inp";
        } else {
            tmp = "test\\project_01_01\\test_" + std::to_string(i) + ".inp";
        }
        std::string str = readFile(tmp);
        BigInt n(parse_little_endian_hex(str));
        n.print_hex();
    }
    return 0;
}