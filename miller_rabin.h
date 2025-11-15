#ifndef MILLER_RABIN_H
#define MILLER_RABIN_H

#include "bigInt.h"
#include <random>    // For std::mt19937_64

/**
 * @brief Performs modular exponentiation (base^exp) % mod.
 */
BigInt powMod(BigInt base, BigInt exp, const BigInt& mod) {
    BigInt result(1);
    base %= mod;
    while (exp > 0) {
        if (!exp.is_even()) {
            result = (result * base) % mod;
        }
        exp = exp >> 1; // exp /= 2
        base = (base * base) % mod;
    }
    return result;
}

/**
 * @brief Generates a cryptographically secure random BigInt in [min, max].
 */
BigInt random_bigint_in_range(const BigInt& min, const BigInt& max) {
    // Create a static generator to be seeded only once
    static std::random_device rd;
    static std::mt19937_64 gen(rd());

    BigInt range = max - min + BigInt(1);
    if (range <= BigInt(0)) {
        return min;
    }

    // This is why bit_length() must be public
    size_t bits = range.bit_length();
    size_t num_limbs = (bits + 63) / 64;
    if (num_limbs == 0) num_limbs = 1;

    BigInt rnd;
    do {
        // .limbs is public, so we can access it
        rnd.limbs.resize(num_limbs, 0);
        for (size_t i = 0; i < num_limbs; ++i) {
            rnd.limbs[i] = gen();
        }

        // Mask off extra bits in the most significant limb
        size_t bits_in_msl = bits % 64;
        if (bits_in_msl > 0) {
            // Create mask, e.g., 0...011111 for 5 bits
            uint64_t mask = (1ULL << bits_in_msl) - 1;
            if (bits_in_msl == 63) mask = (1ULL << 63) - 1 + (1ULL << 63); // Handle 64-bit case
            if (bits_in_msl == 0) mask = ~0ULL; // Handle 64-bit case

            rnd.limbs.back() &= mask;
        }
        
        rnd.normalize();
    } while (rnd >= range); // Rejection sampling

    return rnd + min;
}

/**
 * @brief Checks if the number is probably prime using Miller-Rabin.
 */
bool is_prime_miller_rabin(const BigInt& n, int k) {
    // --- Step 1: Handle edge cases ---
    if (n < BigInt(2)) return false;
    if (n == BigInt(2) || n == BigInt(3)) return true;
    if (n.is_even()) return false;

    // --- Step 2: Find d and r such that n-1 = d * 2^r ---
    BigInt n_minus_1 = n - BigInt(1);
    BigInt d = n_minus_1;
    size_t r = 0;
    while (d.is_even()) {
        d = d >> 1;
        r++;
    }
    
    BigInt n_minus_2 = n - BigInt(2); // For random range [2, n-2]

    // --- Step 3: Witness loop (k rounds) ---
    for (int i = 0; i < k; ++i) {
        // Pick a random base 'a' in the range [2, n-2]
        // Calls the free function
        BigInt a = random_bigint_in_range(BigInt(2), n_minus_2);

        // --- Step 4: Compute x = a^d % n ---
        // Calls the free function
        BigInt x = powMod(a, d, n);

        // If x is 1 or n-1, it might be prime. Continue to next witness.
        if (x == BigInt(1) || x == n_minus_1) {
            continue;
        }

        // --- Step 5: Squaring loop (r-1 times) ---
        bool probably_prime = false;
        for (size_t j = 0; j < r - 1; ++j) {
            x = (x * x) % n;
            
            if (x == n_minus_1) {
                probably_prime = true;
                break;
            }
        }

        // --- Step 6: Verdict for this witness ---
        if (!probably_prime) {
            return false;
        }
    }

    // --- Step 7: Final verdict ---
    return true;
}

#endif // MILLER_RABIN_H