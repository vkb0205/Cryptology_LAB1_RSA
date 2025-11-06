#include <iostream> // Dùng cho std::cin, std::cout, std::endl
#include <string>   // Dùng cho std::string
#include <sstream>  // Dùng cho std::stringstream
#include <iomanip>  // Dùng cho std::hex, std::uppercase, std::setfill, std::setw
#include "bigInt.h"

// --- Bezout Algorithm ---

SignedBigInt bezout(const SignedBigInt& a, const SignedBigInt& b, SignedBigInt& x, SignedBigInt& y) {
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



int main(){
    SignedBigInt a(BigInt("65"));
    SignedBigInt b(BigInt("17"));
    SignedBigInt one(BigInt(1));
    SignedBigInt phi = (a - one) * (b - one);
    std::cout << "a - 1: " << (a - one).to_hex_string() << std::endl;
    std::cout << "b - 1: " << (b - one).to_hex_string() << std::endl;
    std::cout << "a: " << a.to_hex_string() << std::endl;
    std::cout << "b: " << b.to_hex_string() << std::endl;
    std::cout << "phi: " << phi.to_hex_string() << std::endl;
    SignedBigInt e(BigInt("21"));
    SignedBigInt x, y;
    SignedBigInt gcd = bezout(e, phi, x, y);
    std::cout << "original x: " << x.to_hex_string() << std::endl;
    x = ((x % phi) + phi) % phi;
    std::cout << "GCD: " << gcd.to_hex_string() << std::endl;
    std::cout << "x: " << x.to_hex_string() << std::endl;
    std::cout << "y: " << y.to_hex_string() << std::endl;
    return 0;
}