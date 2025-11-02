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
// We need a 128-bit type to multiply two 64-bit numbers.
// C++ __uint128_t is non-standard but supported by GCC/Clang.
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

    // Addition (a + b)
    friend BigInt operator+(const BigInt& a, const BigInt& b) {
        
    }

    // Subtraction (a - b)
    friend BigInt operator-(const BigInt& a, const BigInt& b) {
        
    }

    // Multiplication (a * b) - Grade School O(n^2)
    friend BigInt operator*(const BigInt& a, const BigInt& b) {
        
    }
    
    // Division and Modulo (a / b) and (a % b)
    // Returns {quotient, remainder}
    static std::pair<BigInt, BigInt> divmod(const BigInt& dividend_in, const BigInt& divisor_in) {
        
    }

    friend BigInt operator/(const BigInt& a, const BigInt& b) {
        return divmod(a, b).first;
    }
    friend BigInt operator%(const BigInt& a, const BigInt& b) {
        return divmod(a, b).second;
    }
};

// --- Layer 3: The Algorithms ---

/**
 * @brief Modular Exponentiation: (base ^ exp) % mod
 * Uses exponentiation by squaring.
 */
BigInt modular_pow(BigInt base, BigInt exp, const BigInt& mod) {
    BigInt result(1);
    base = base % mod;
    while (!exp.is_zero()) {
        // If exponent is odd, multiply result by base
        if (!exp.is_even()) {
            result = (result * base) % mod;
        }
        // Exponent is now even, divide by 2
        exp = exp >> 1;
        // Square the base
        base = (base * base) % mod;
    }
    return result;
}

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


// --- Main Program ---
int main() {
    std::ifstream infile("test.inp");
    if (!infile.is_open()) {
        std::cerr << "Error: Could not open test.inp" << std::endl;
        return 1;
    }

    std::string le_hex_str;
    std::getline(infile, le_hex_str);
    infile.close();

    if (le_hex_str.empty()) {
        std::cerr << "Error: test.inp is empty." << std::endl;
        return 1;
    }

    try {
        // 1. Parse the little-endian string
        std::string be_hex_str = parse_little_endian_hex(le_hex_str);

        // 2. Load into our BigInt class
        BigInt number_to_test(be_hex_str);

        // 3. Run the primality test
        // if (is_prime(number_to_test)) {
        //     std::cout << "Prime" << std::endl;
        // } else {
        //     std::cout << "Composite" << std::endl;
        // }

        

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}