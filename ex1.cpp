#include "helper.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
using namespace std;


/* ================= MILLER–RABIN ================= */

bool isComposite(const string &n, const string &a, const string &d, int r) {
    return 1;
}

bool millerRabin(const string &n, int k = 5) {
    return 1;
}


int main(){
    string n;
    cout << "Enter big number (up to 512-bit): ";
    cin >> n;

    if (millerRabin(n))
        cout << "Probably prime.\n";
    else
        cout << "Composite.\n";
    return 0;
}