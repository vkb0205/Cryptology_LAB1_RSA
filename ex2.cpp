#include <iostream> // Dùng cho std::cin, std::cout, std::endl
#include <string>   // Dùng cho std::string
#include <sstream>  // Dùng cho std::stringstream
#include <iomanip>  // Dùng cho std::hex, std::uppercase, std::setfill, std::setw
#include "bigInt.h"
#include <tuple>
#include <fstream>

// --- Bezout Algorithm ---

SignedBigInt bezout(const SignedBigInt& a, const SignedBigInt& b, SignedBigInt& x, SignedBigInt& y) {
    //a = e, b = phi, x = d
    //Bezout: ax + by = gcd(a, b)
    SignedBigInt m0 = a, n0 = b;
    SignedBigInt x0 = SignedBigInt(1), y0 = SignedBigInt(0);
    SignedBigInt x1 = SignedBigInt(0), y1 = SignedBigInt(1);
    while (n0 != SignedBigInt(0)) {
        SignedBigInt q = m0 / n0;
        SignedBigInt r = m0 % n0;
        SignedBigInt xr = x0 - q * x1, yr = y0 - q * y1;
        m0 = n0;
        n0 = r;
        x0 = x1;
        y0 = y1;
        x1 = xr;
        y1 = yr;
    }
    x = x0;
    y = y0;
    return m0;
}

std::tuple<SignedBigInt, SignedBigInt, SignedBigInt> readFile_ex2(std::string filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Cannot read file: " << filename << std::endl;
        return {};
    }
    
    std::string p_str, q_str, e_str;
    file >> p_str >> q_str >> e_str;
    
    // Convert from little-endian hex to BigInt
    BigInt p(parse_little_endian_hex(p_str));
    BigInt q(parse_little_endian_hex(q_str));
    BigInt e(parse_little_endian_hex(e_str));
    
    file.close();
    
    return std::make_tuple(SignedBigInt(p), SignedBigInt(q), SignedBigInt(e));
}



int main(){
    for (int i = 0; i < 9; i++) {
        std::string path = "test\\project_01_02\\test_0" + std::to_string(i) + ".inp";
        auto [p, q, e] = readFile_ex2(path);
        // SignedBigInt a(BigInt("65"));
        // SignedBigInt b(BigInt("17"));
        SignedBigInt one(BigInt(1));
        SignedBigInt phi = (p - one) * (q - one);
        std::cout << "a - 1: " << (p - one).to_hex_string() << std::endl;
        std::cout << "b - 1: " << (q - one).to_hex_string() << std::endl;
        std::cout << "p: " << p.to_hex_string() << std::endl;
        std::cout << "q: " << q.to_hex_string() << std::endl;
        std::cout << "phi: " << phi.to_hex_string() << std::endl;
        SignedBigInt x, y;
        SignedBigInt gcd = bezout(e, phi, x, y);
        std::cout << "original x: " << x.to_hex_string() << std::endl;
        x = ((x % phi) + phi) % phi;
        std::cout << "GCD: " << gcd.to_hex_string() << std::endl;
        std::cout << "x: " << x.to_hex_string() << std::endl;
        std::cout << "y: " << y.to_hex_string() << std::endl;
        std::cout << "\n=============\n";
    }
    
    
    return 0;
}