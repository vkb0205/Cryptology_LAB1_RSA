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
#if defined(__GNUC__) || defined(__clang__)
using uint128_t = __uint128_t;
#else
// Fallback for MSVC or other compilers (much slower)
struct uint128_t {
    uint64_t lo, hi;
    uint128_t(uint64_t l = 0, uint64_t h = 0) : lo(l), hi(h) {}
    // Simplified constructor for this problem
    uint128_t(uint64_t l) : lo(l), hi(0) {}
};
#endif


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

    friend BigInt operator+(const BigInt& a, const BigInt& b) {
        BigInt result;
        size_t n = std::max(a.limbs.size(), b.limbs.size());
        result.limbs.resize(n);
        
        uint64_t carry = 0;
        for (size_t i = 0; i < n; ++i) {
            uint64_t a_limb = (i < a.limbs.size()) ? a.limbs[i] : 0;
            uint64_t b_limb = (i < b.limbs.size()) ? b.limbs[i] : 0;
            
            // This detects overflow: if sum < a_limb, it wrapped around
            uint64_t sum = a_limb + carry;
            bool carry1 = (sum < a_limb);
            
            sum += b_limb;
            bool carry2 = (sum < b_limb);
            
            result.limbs[i] = sum;
            carry = (carry1 || carry2) ? 1 : 0;
        }
        if (carry > 0) {
            result.limbs.push_back(carry);
        }
        return result;
    }

    // Subtraction (a - b)
    friend BigInt operator-(const BigInt& a, const BigInt& b) {
        if (a < b) throw std::runtime_error("Subtraction underflow");
        BigInt result = a;
        uint64_t borrow = 0;
        for (size_t i = 0; i < b.limbs.size() || borrow; ++i) {
            uint64_t b_limb = (i < b.limbs.size()) ? b.limbs[i] : 0;
            
            uint64_t diff = result.limbs[i] - borrow;
            bool borrow1 = (diff > result.limbs[i]); // Underflow
            
            result.limbs[i] = diff - b_limb;
            bool borrow2 = (result.limbs[i] > diff); // Underflow
            
            borrow = (borrow1 || borrow2) ? 1 : 0;
        }
        result.normalize();
        return result;
    }

    // Multiplication (a * b) - Grade School O(n^2)
    // friend BigInt operator*(const BigInt& a, const BigInt& b) {
    //     if (a.is_zero() || b.is_zero()) return BigInt(0);
        
    //     BigInt result;
    //     result.limbs.resize(a.limbs.size() + b.limbs.size(), 0);

    //     for (size_t i = 0; i < a.limbs.size(); ++i) {
    //         uint64_t carry = 0;
    //         for (size_t j = 0; j < b.limbs.size() || carry; ++j) {
    //             uint64_t b_limb = (j < b.limbs.size()) ? b.limbs[j] : 0;
                
    //             // Use 128-bit math to hold the product
    //             uint128_t product = (uint128_t)a.limbs[i] * b_limb + 
    //                                 result.limbs[i + j] + carry;
                
    //             result.limbs[i + j] = (uint64_t)product; // Lower 64 bits
    //             carry = (uint64_t)(product >> 64);      // Upper 64 bits
    //         }
    //     }
    //     result.normalize();
    //     return result;
    // }
    
    // Division and Modulo (a / b) and (a % b)
    // Returns {quotient, remainder}
    static std::pair<BigInt, BigInt> divmod(const BigInt& dividend_in, const BigInt& divisor_in) {
        if (divisor_in.is_zero()) throw std::runtime_error("Division by zero");
        
        BigInt quotient(0);
        BigInt remainder(0);
        BigInt dividend = dividend_in;
        BigInt divisor = divisor_in;

        if (dividend < divisor) {
            return {BigInt(0), dividend};
        }

        // Binary Long Division (a simpler, but not fastest, approach)
        // Find the highest power of 2 to multiply the divisor by
        BigInt temp_divisor = divisor;
        BigInt power_of_two(1);
        
        while (temp_divisor <= dividend) {
            temp_divisor = temp_divisor << 1;
            power_of_two = power_of_two << 1;
        }
        
        // Now walk back down
        temp_divisor = temp_divisor >> 1;
        power_of_two = power_of_two >> 1;

        remainder = dividend;
        while (!power_of_two.is_zero()) {
            if (remainder >= temp_divisor) {
                remainder = remainder - temp_divisor;
                quotient = quotient + power_of_two;
            }
            temp_divisor = temp_divisor >> 1;
            power_of_two = power_of_two >> 1;
        }

        quotient.normalize();
        remainder.normalize();
        return {quotient, remainder};
    }

    friend BigInt operator/(const BigInt& a, const BigInt& b) {
        return divmod(a, b).first;
    }
    friend BigInt operator%(const BigInt& a, const BigInt& b) {
        return divmod(a, b).second;
    }

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
    }
};

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
    std::string str1 = readFile("test\\project_01_01\\test_00.inp");
    std::string str2 = readFile("test\\project_01_01\\test_01.inp");
    if (str1.empty()) {
        std::cerr << "Error: test.inp is empty." << std::endl;
        return 1;
    }
    BigInt n(parse_little_endian_hex(str1));
    BigInt m(parse_little_endian_hex(str2));

    n = n + m;
    n.print_hex();

    return 0;
}