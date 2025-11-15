#include "../bigInt.h"

// --- Bezout Algorithm ---
BigInt bezout(const BigInt& a, const BigInt& b, BigInt& x, BigInt& y) {
    BigInt m0 = a, n0 = b;
    BigInt x0 = BigInt(1), y0 = BigInt(0);
    BigInt x1 = BigInt(0), y1 = BigInt(1);
    while (n0 != BigInt(0)) {
        BigInt q = m0 / n0;
        BigInt r = m0 % n0;
        BigInt xr = x0 - q * x1, yr = y0 - q * y1;
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

    std::string p_str, q_str, e_str;
    file >> p_str >> q_str >> e_str;
    file.close();

    // Convert from hex string to BigInt
    BigInt p(std::string(p_str.rbegin(), p_str.rend()));
    BigInt q(std::string(q_str.rbegin(), q_str.rend()));
    BigInt e(std::string(e_str.rbegin(), e_str.rend()));

    BigInt one(BigInt(1));
    BigInt phi = (p - one) * (q - one);

    BigInt x, y;
    BigInt gcd = bezout(e, phi, x, y);

    if (gcd != BigInt(1)) {
        outfile << "-1" << std::endl;
    }
    else {
        x = ((x % phi) + phi) % phi; // Ensure x is positive

        std::string result = x.to_hex_string();
        result = std::string(result.rbegin(), result.rend()); // Reverse the string

        outfile << result << std::endl;
    }
    outfile.close();

    return 0;
}