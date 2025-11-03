#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <random>
#include "bigInt.h"


std::tuple<uint64_t, uint64_t> decompose(uint64_t n_minus_1) {
    uint64_t s = 0;
    uint64_t d = n_minus_1;
    while ((d & 1ULL) == 0ULL) {
        ++s;
        d >>= 1;
    }
    return std::make_tuple(s, d);
}
static inline uint64_t mulMod(uint64_t a, uint64_t b, uint64_t mod) {
    return static_cast<uint64_t>((__uint128_t)a * b % mod);
}

uint64_t powMod(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1ULL;
    if (mod == 1ULL) return 0ULL;
    base %= mod;
    while (exp > 0) {
        if (exp & 1ULL) res = mulMod(res, base, mod);
        base = mulMod(base, base, mod);
        exp >>= 1ULL;
    }
    return res;
}

bool isWitness(uint64_t a, uint64_t n, uint64_t s, uint64_t d) {
    uint64_t x = powMod(a, d, n);
    if (x == 1ULL || x == n - 1ULL) return false;
    // Loop s - 1 times
    for (uint64_t i = 1; i < s; ++i) {
        x = mulMod(x, x, n);
        if (x == n - 1ULL) return false;
    }
    return true; // composite witness
}


static std::vector<uint64_t> chooseBases(uint64_t n, int rounds, bool deterministic)
{
    std::vector<uint64_t> bases;

    // Caller should handle n <= 4
    if (n <= 4) return bases;

    if (deterministic || rounds <= 0) {
        // Deterministic sets
        if (n < 4294967296ULL) {                 // n < 2^32
            bases = { 2ULL, 7ULL, 61ULL };
        } else if (n < 3474749660383ULL) {
            bases = { 2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL };
        } else if (n < 341550071728321ULL) {
            bases = { 2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL };
        } else {                                  // all 64-bit n
            bases = { 2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL };
        }

        // Drop any base >= n (edge cases when n is tiny)
        bases.erase(
            std::remove_if(bases.begin(), bases.end(), [n](uint64_t a){ return a >= n; }),
            bases.end()
        );
    } else {
        // Probabilistic: pick unique random bases in [2, n-2]
        uint64_t maxDistinct = (n > 4) ? (n - 3) : 0; // count in [2, n-2]
        if (rounds < 0) rounds = 0;
        if (static_cast<uint64_t>(rounds) > maxDistinct)
            rounds = static_cast<int>(maxDistinct);

        std::random_device rd;
        std::mt19937_64 rng(rd());
        std::uniform_int_distribution<uint64_t> dist(2ULL, n - 2ULL);

        bases.reserve(rounds);
        while (static_cast<int>(bases.size()) < rounds) {
            uint64_t a = dist(rng);
            if (std::find(bases.begin(), bases.end(), a) == bases.end())
                bases.push_back(a);
        }
    }

    return bases;
}

bool isPropablePrime(uint64_t n, int rounds) {
    if (n < 2UL) {return false;};
    if (n == 2ULL || n == 3ULL) {return true;};
    if ((n & 1ULL) == 0ULL) {return false;};

    auto [s, d] = decompose(n - 1ULL);

    std::vector<uint64_t> bases = chooseBases(n, rounds, true);

    for (uint64_t a : bases) {
        if (a % n == 0ULL) continue;
        if (isWitness(a, n, s, d)) return false;
    }
    return true;
}

// --- BigInt Miller-Rabin Implementation ---

// Decompose n-1 = 2^s * d
std::tuple<uint64_t, BigInt> decompose_big(BigInt n_minus_1) {
    uint64_t s = 0;
    BigInt d = n_minus_1;
    while (d.is_even()) {
        d = d >> 1;
        s++;
    }
    return std::make_tuple(s, d);
}

// Modular exponentiation: (base^exp) % mod for BigInt
BigInt powMod_big(BigInt base, BigInt exp, const BigInt& mod) {
    BigInt res(1);
    base = base % mod;
    while (!exp.is_zero()) {
        if (!exp.is_even()) {
            res = (res * base) % mod;
        }
        base = (base * base) % mod;
        exp = exp >> 1;
    }
    return res;
}

// Witness test for BigInt
bool isWitness_big(const BigInt& a, const BigInt& n, uint64_t s, const BigInt& d) {
    BigInt x = powMod_big(a, d, n);
    BigInt n_minus_1 = n - BigInt(1);

    if (x == BigInt(1) || x == n_minus_1) {
        return false; // Not a witness
    }

    for (uint64_t i = 1; i < s; ++i) {
        x = (x * x) % n;
        if (x == n_minus_1) {
            return false; // Not a witness
        }
    }
    return true; // Is a witness (n is composite)
}

// Miller-Rabin primality test for BigInt
bool isProbablePrime_big(const BigInt& n, int rounds) {
    if (n < BigInt(2)) return false;
    if (n == BigInt(2) || n == BigInt(3)) return true;
    if (n.is_even()) return false;

    auto [s, d] = decompose_big(n - BigInt(1));

    // For BigInt, we still use small uint64_t bases for efficiency.
    // This is standard practice.
    std::vector<uint64_t> bases = chooseBases(n.to_uint64(), rounds, true);

    for (uint64_t a_val : bases) {
        BigInt a(a_val);
        if (a >= n) continue; // Base must be smaller than n
        if (isWitness_big(a, n, s, d)) {
            return false; // Composite
        }
    }
    return true; // Probably prime
}

// int main() {
//     // --- Test Basic BigInt Arithmetic First ---
//     std::cout << "--- Testing Basic BigInt Operations ---\n";
    
//     // Test 1: Multiplication
//     BigInt a("10000000000000000"); // 2^64
//     BigInt b("2");
//     BigInt product = a * b;
//     std::cout << "Test Mul: 2^64 * 2 = ";
//     product.print_hex();
//     // Expected: 0x20000000000000000
    
//     // Test 2: Modulo with detailed debug
//     std::cout << "\n=== Detailed Debug for 17 % 5 ===\n";
//     BigInt c("11");  // 17
//     BigInt d("5");   // 5
    
//     std::cout << "c (17) = "; c.print_hex();
//     std::cout << "d (5) = "; d.print_hex();
    
//     // Test subtraction directly
//     BigInt test_sub = c - d;
//     std::cout << "17 - 5 = "; test_sub.print_hex();
    
//     BigInt test_sub2 = test_sub - d;
//     std::cout << "12 - 5 = "; test_sub2.print_hex();
    
//     BigInt test_sub3 = test_sub2 - d;
//     std::cout << "7 - 5 = "; test_sub3.print_hex();
    
//     auto [q, r] = BigInt::divmod(c, d);
//     std::cout << "Debug: 17 / 5 = quotient: ";
//     q.print_hex();
//     std::cout << "Debug: 17 / 5 = remainder: ";
//     r.print_hex();
//     BigInt mod = c % d;
//     std::cout << "Test Mod: 17 % 5 = ";
//     mod.print_hex();
//     std::cout << "===================================\n\n";
// // ...existing code...
    
//     // Test 3: Power Mod
//     BigInt base("2");
//     BigInt exp("A");
//     BigInt modulus("3E8");
//     BigInt result = powMod_big(base, exp, modulus);
//     std::cout << "Test PowMod: 2^10 % 1000 = ";
//     result.print_hex();
//     // Expected: 0x18 (24 decimal, since 1024 % 1000 = 24)
    
//     std::cout << "\n";

//     // --- Test with BigInt ---
//     std::cout << "--- Testing BigInt Miller-Rabin ---\n";
// // ...existing code...
//     return 0;
// }

int main() {
    struct Case { BigInt n; bool prime; };
    std::vector<Case> cases = {
        { BigInt(2),  true },
        { BigInt(3),  true },
        { BigInt(4),  false },
        { BigInt(0x11), true },           // 17 decimal
        { BigInt(0x15), false },          // 21 decimal
        { BigInt(0x17), true },           // 23 decimal
        { BigInt("3B"), true },           // 0x3B = 59
        { BigInt("1FFFFFFFFFFFFFFF"), true }, // 2^61 − 1 (hex)
        { BigInt("FEE1A8B523211E7342A8863D2632D2F422525F206C730D91293A1439983335BB"), true },
        { BigInt("E152201326324E8F2994496A4E879D24E4874D601A03FE46A543CD1499D06F41"), false }
    };

    int failures = 0;
    for (const auto& c : cases) {
        bool got = isProbablePrime_big(c.n, /*rounds=*/0);
        if (got != c.prime) {
            std::cout << "FAIL "
                      << (c.n.is_zero() ? "0"
                                        : c.n.to_hex_string())
                      << " expected " << (c.prime ? "prime" : "composite")
                      << " got " << (got ? "prime" : "composite") << "\n";
            ++failures;
        }
    }

    if (failures == 0) {
        std::cout << "All BigInt Miller-Rabin tests passed (" << cases.size() << ")\n";
    } else {
        std::cout << failures << " tests failed\n";
    }
    return failures ? 1 : 0;
}