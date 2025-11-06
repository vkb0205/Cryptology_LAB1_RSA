#include "bigInt.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <string>
#include <fstream>
#include "miller_rabin.h"
using namespace std;


/* ================= MILLER–RABIN ================= */
string readInput(string filename) {
    ifstream file(filename);
    if (!file) {
        cout << "Cannot read file" << filename << "\n";
        return "";
    }

    string buffer; 
    file >> buffer;

    file.close();
    return buffer;
}

BigInt convertToBigInt(string hex) {
    return BigInt(parse_little_endian_hex(hex));
}

bool validPrime(BigInt& n) {
    return isProbablePrime_big(n, 10);
}


int main(){
    // for (int i = 0; i < 10; i++) {
    //     string path = string("test/project_01_01/test_0") + to_string(i) + ".inp";
    //     cout << path << endl;
    //     string input = readInput(path);
    //     cout << input << endl;
    //     BigInt num = convertToBigInt(input);
    //     cout << "decoded = ";
    //     num.print_hex();
    //     if (validPrime(num)) {
    //         cout << "1";
    //     }
    //     else {
    //         cout << "0";
    //     }
    //     cout << endl << endl;
    // }

    // 512-bit hex (128 characters)
    string test_512 = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    BigInt big = BigInt(test_512);
    cout << "Is prime? " << isProbablePrime_big(big, 10) << endl;
    
    return 0;
}

//TODO: 
/*
If you need speed for 512-bit+:

Implement Karatsuba multiplication
Use Montgomery reduction for modular arithmetic
Or switch to a library like GMP
Bottom line: Your code is correct for 512-bit, just not fast.
*/