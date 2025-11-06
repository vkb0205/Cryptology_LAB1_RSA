#include "miller_rabin.h"

int main() {
    // --- Test Basic BigInt Arithmetic First ---
    std::cout << "--- Testing Basic BigInt Operations ---\n";
    
    // Test 1: Multiplication
    BigInt a("10000000000000000"); // 2^64
    BigInt b("2");
    BigInt product = a * b;
    std::cout << "Test Mul: 2^64 * 2 = ";
    product.print_hex();
    // Expected: 0x20000000000000000
    
    // Test 2: Modulo with detailed debug
    std::cout << "\n=== Detailed Debug for 17 % 5 ===\n";
    BigInt c("11");  // 17
    BigInt d("5");   // 5
    
    std::cout << "c (17) = "; c.print_hex();
    std::cout << "d (5) = "; d.print_hex();
    
    // Test subtraction directly
    BigInt test_sub = c - d;
    std::cout << "17 - 5 = "; test_sub.print_hex();
    
    BigInt test_sub2 = test_sub - d;
    std::cout << "12 - 5 = "; test_sub2.print_hex();
    
    BigInt test_sub3 = test_sub2 - d;
    std::cout << "7 - 5 = "; test_sub3.print_hex();
    
    auto [q, r] = BigInt::divmod(c, d);
    std::cout << "Debug: 17 / 5 = quotient: ";
    q.print_hex();
    std::cout << "Debug: 17 / 5 = remainder: ";
    r.print_hex();
    BigInt mod = c % d;
    std::cout << "Test Mod: 17 % 5 = ";
    mod.print_hex();
    std::cout << "===================================\n\n";
// ...existing code...
    
    // Test 3: Power Mod
    BigInt base("2");
    BigInt exp("A");
    BigInt modulus("3E8");
    BigInt result = powMod_big(base, exp, modulus);
    std::cout << "Test PowMod: 2^10 % 1000 = ";
    result.print_hex();
    // Expected: 0x18 (24 decimal, since 1024 % 1000 = 24)
    
    std::cout << "\n";

    // --- Test with BigInt ---
    std::cout << "--- Testing BigInt Miller-Rabin ---\n";
// ...existing code...
    return 0;
}

// int main() {
//     struct Case { BigInt n; bool prime; };
//     std::vector<Case> cases = {
//         { BigInt(2),  true },
//         { BigInt(3),  true },
//         { BigInt(4),  false },
//         { BigInt(0x11), true },           // 17 decimal
//         { BigInt(0x15), false },          // 21 decimal
//         { BigInt(0x17), true },           // 23 decimal
//         { BigInt("3B"), true },           // 0x3B = 59
//         { BigInt("1FFFFFFFFFFFFFFF"), true }, // 2^61 − 1 (hex)
//         { BigInt("FEE1A8B523211E7342A8863D2632D2F422525F206C730D91293A1439983335BB"), true },
//         { BigInt("E152201326324E8F2994496A4E879D24E4874D601A03FE46A543CD1499D06F41"), false }
//     };

//     int failures = 0;
//     for (const auto& c : cases) {
//         bool got = isProbablePrime_big(c.n, /*rounds=*/0);
//         if (got != c.prime) {
//             std::cout << "FAIL "
//                       << (c.n.is_zero() ? "0"
//                                         : c.n.to_hex_string())
//                       << " expected " << (c.prime ? "prime" : "composite")
//                       << " got " << (got ? "prime" : "composite") << "\n";
//             ++failures;
//         }
//     }

//     if (failures == 0) {
//         std::cout << "All BigInt Miller-Rabin tests passed (" << cases.size() << ")\n";
//     } else {
//         std::cout << failures << " tests failed\n";
//     }
//     return failures ? 1 : 0;
// }