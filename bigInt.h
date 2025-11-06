// helper.h - declarations for big-integer helpers
#ifndef HELPER_H
#define HELPER_H

#include <string>

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

    explicit operator uint64_t() const {
        return lo;
    };

    static inline uint128_t mul_u64(uint64_t a, uint64_t b) {
        unsigned __int64 hi;
        unsigned __int64 lo = _umul128(a, b, &hi); 
        return uint128_t(lo, hi);
    }

    // Helper to add a 128-bit and 64-bit integer
    static inline uint128_t add_u128_u64(uint128_t a, uint64_t b) {
        uint64_t new_lo = a.lo + b;
        uint64_t carry = (new_lo < a.lo); // 1 if overflow occurred
        return uint128_t(new_lo, a.hi + carry);
    }
};
#endif


/**
 * @brief Layer 1: The BigInt Class
 * Stores a large unsigned integer as a vector of 64-bit "limbs".
 * Limbs are little-endian (limbs[0] is the least significant).
 */
class BigInt {
private: 
    static unsigned count_leading_zeros(uint64_t limb) {
    #if defined(_MSC_VER) && !defined(__clang__)
        unsigned long idx;
        _BitScanReverse64(&idx, limb);
        return 63u - idx;
    #else
        return limb ? static_cast<unsigned>(__builtin_clzll(limb)) : 64u;
    #endif
    }

    size_t bit_length() const {
        if (is_zero()) return 0;
        size_t ms = limbs.size() - 1;
        uint64_t v = limbs[ms];
        return ms * 64 + (64 - count_leading_zeros(v));
    }
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

        //Since the string stores the number in the format of big-endian
        for (int i = len - 1; i >= 0; --i) {
            char c = hex_str[i];
            uint64_t val;
            if (c >= '0' && c <= '9') val = c - '0';
            else if (c >= 'a' && c <= 'f') 
                val = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') 
                val = c - 'A' + 10;
            else 
                throw std::runtime_error("Invalid hex character");

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

    uint64_t to_uint64() const {
        if (limbs.empty()) {
            return 0;
        }
        // If there's more than one limb, the number is larger than 2^64 - 1.
        if (limbs.size() > 1) {
            return UINT64_MAX;
        }
        // Otherwise, the value is just the single limb.
        return limbs[0];
    }

    std::string to_hex_string() const {
        std::ostringstream oss;
        oss << std::hex << std::nouppercase;
        if (limbs.empty()) return "0";
        oss << limbs.back();
        for (size_t i = limbs.size(); i-- > 1;) {
            oss << std::setw(16) << std::setfill('0') << limbs[i - 1];
        }
        return oss.str();
    }

    // --- Helper Functions ---
    //Trimming leading zero limbs
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
        //130 / 64 = 2, r = 2
        size_t shift_limbs = shift_bits / 64;
        size_t inner_shift = shift_bits % 64;

        if (inner_shift > 0) {
            uint64_t carry = 0;
            for (size_t i = 0; i < result.limbs.size(); ++i) {
                uint64_t next_carry = result.limbs[i] >> (64 - inner_shift); //Take the overflow when you shift left 
                result.limbs[i] = (result.limbs[i] << inner_shift) | carry; //Store the rest except the overflow part into the current limb
                carry = next_carry; //Take the overflow part to the next limb
            }
            if (carry > 0) 
                result.limbs.push_back(carry);
        }
        if (shift_limbs > 0) {
            result.limbs.insert(result.limbs.begin(), shift_limbs, 0); //Insert 0s to the least significant location
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
            /*
            a =    |||| ||||
            b = || |||| ||||
            */
           //Take each set of 64-bit in each limb, if exceed the smaller bigint, refresh the index of that to 0 and go on
            uint64_t a_limb = (i < a.limbs.size()) ? a.limbs[i] : 0;
            uint64_t b_limb = (i < b.limbs.size()) ? b.limbs[i] : 0;
            
            // This detects overflow: if sum < a_limb, it wrapped around
            uint64_t sum = a_limb + carry; //sum is always > a_limb
            bool carry1 = (sum < a_limb);  //if sum is < a_limb ==> overflow
            
            sum += b_limb;
            bool carry2 = (sum < b_limb);
            
            result.limbs[i] = sum;
            carry = (carry1 || carry2) ? 1 : 0;
        }
        if (carry > 0) {
            result.limbs.push_back(carry); //0x00000000000000001
        }
        return result;
    }

    // Subtraction (a - b)
    friend BigInt operator-(const BigInt& a, const BigInt& b) {
        if (a < b) {
            throw std::underflow_error("BigInt subtraction would result in negative value");
        }
        
        BigInt result = a;
        uint64_t borrow = 0;
        
        for (size_t i = 0; i < b.limbs.size() || borrow; ++i) {
            uint64_t b_limb = (i < b.limbs.size()) ? b.limbs[i] : 0;
            
            // Check if we need to borrow
            if (result.limbs[i] < b_limb + borrow) {
                result.limbs[i] = result.limbs[i] + (UINT64_MAX - b_limb - borrow + 1);
                borrow = 1;
            } else {
                result.limbs[i] = result.limbs[i] - b_limb - borrow;
                borrow = 0;
            }
        }
        
        result.normalize();
        return result;
    }

    //Multiplication (a * b) - Grade School O(n^2)
    friend BigInt operator*(const BigInt& a, const BigInt& b) {
        if (a.is_zero() || b.is_zero()) return BigInt(0);
        
        BigInt result;
        result.limbs.assign(a.limbs.size() + b.limbs.size(), 0);

        for (size_t i = 0; i < a.limbs.size(); ++i) {
            uint64_t carry = 0;
            for (size_t j = 0; j < b.limbs.size(); ++j) {
                #if defined(__GNUC__) || defined(__clang__)
                // GCC/Clang path using native 128-bit type
                __uint128_t product = (__uint128_t)a.limbs[i] * b.limbs[j] + 
                                      result.limbs[i + j] + carry;
                
                result.limbs[i + j] = (uint64_t)product;
                carry = (uint64_t)(product >> 64);
                #else
                // MSVC path using the struct and static helpers
                uint128_t product = uint128_t::mul_u64(a.limbs[i], b.limbs[j]);
                product = uint128_t::add_u128_u64(product, result.limbs[i + j]);
                product = uint128_t::add_u128_u64(product, carry);

                result.limbs[i + j] = product.lo;
                carry = product.hi;
                #endif
            }
            if (carry > 0) {
                result.limbs[i + b.limbs.size()] += carry;
            }
        }
        result.normalize();
        return result;
    }

    
    // Division and Modulo (a / b) and (a % b)
    // Returns {quotient, remainder}
    // static std::pair<BigInt, BigInt> divmod(const BigInt& dividend, const BigInt& divisor) {
    //     if (divisor.is_zero()) {
    //         throw std::invalid_argument("Division by zero");
    //     }
    //     if (dividend < divisor) {
    //         return {BigInt(0), dividend};
    //     }

    //     BigInt remainder = dividend;
    //     BigInt quotient(0);

    //     size_t shift = remainder.bit_length() - divisor.bit_length();
    //     BigInt shifted = divisor << shift;

    //     for (size_t i = shift + 1; i-- > 0;) {
    //         if (!(remainder < shifted)) {
    //             remainder = remainder - shifted;
    //             quotient = quotient + (BigInt(1) << i);
    //         }
    //         shifted = shifted >> 1;
    //     }
    //     remainder.normalize();
    //     quotient.normalize();
    //     return {quotient, remainder};
    // }
    static std::pair<BigInt, BigInt> divmod(const BigInt& dividend_in, const BigInt& divisor_in) {
        if (divisor_in.is_zero()) 
            throw std::runtime_error("Division by zero");
        //Res
        BigInt quotient(0);
        BigInt remainder(0);
        //Make copy
        BigInt dividend = dividend_in;
        BigInt divisor = divisor_in;

        if (dividend < divisor) {
            return {BigInt(0), dividend};
        }

        // Binary Long Division (a simpler, but not fastest, approach)
        // Find the highest power of 2 to multiply the divisor by
        // divisor = 2^s . d 
            //d -> odd
            //s -> N
        BigInt temp_divisor = divisor;
        BigInt power_of_two(1); //0000 0001
        
        while (temp_divisor <= dividend) {
            temp_divisor = temp_divisor << 1; //Find biggest even num (= 2^x * divisor) only after (>) the dividend 
            power_of_two = power_of_two << 1; //2^x
        }
        
        // Now walk back down
        temp_divisor = temp_divisor >> 1;
        power_of_two = power_of_two >> 1;

        //Dividend = 2^x * divisor + remainder
        //           |               |
        //           --> quotient    --> remainder
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
// std::string parse_little_endian_hex(const std::string& le_hex) {
//     if (le_hex.length() % 2 != 0) {
//         throw std::runtime_error("Input hex string has odd length.");
//     }
//     std::string be_hex;
//     // Reserve space for the new string
//     be_hex.reserve(le_hex.length());
    
//     // Iterate from the end, taking 2-char (byte) chunks
//     for (int i = le_hex.length() - 2; i >= 0; i -= 2) {
//         be_hex.push_back(le_hex[i]);
//         be_hex.push_back(le_hex[i + 1]);
//     }
//     //DEBUG
//     std::cout << "DEBUG: LE '" << le_hex << "' --> BE '" << be_hex << "'\n";  // ← ADD THIS
//     return be_hex;
// }

//Newer version for parser
std::string parse_little_endian_hex(const std::string& le_hex) {
    if (le_hex.length() % 2 != 0) {
        throw std::runtime_error("Input hex string has odd length.");
    }
    
    // Simply reverse the entire string
    std::string be_hex(le_hex.rbegin(), le_hex.rend());
    
    std::cout << "DEBUG: LE '" << le_hex << "' --> BE '" << be_hex << "'\n";
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

#endif // HELPER_H
