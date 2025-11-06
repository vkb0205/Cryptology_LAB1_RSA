#include <iostream> // Dùng cho std::cin, std::cout, std::endl
#include <string>   // Dùng cho std::string
#include <sstream>  // Dùng cho std::stringstream
#include <iomanip>  // Dùng cho std::hex, std::uppercase, std::setfill, std::setw

/**
 * @brief Tính toán Thuật toán Euclid Mở rộng.
 * * Tìm x và y sao cho ax + by = gcd(a, b).
 * Trong bối cảnh của RSA, chúng ta tìm d và k sao cho ed + k*phi = gcd(e, phi).
 * * @param a Số thứ nhất (e trong RSA)
 * @param b Số thứ hai (phi trong RSA)
 * @param x Tham chiếu để lưu trữ hệ số của a (sẽ là d)
 * @param y Tham chiếu để lưu trữ hệ số của b (sẽ là k)
 * @return long long Giá trị gcd(a, b).
 */
long long extendedEuclidean(long long a, long long b, long long &x, long long &y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    long long x1, y1;
    long long gcd = extendedEuclidean(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return gcd;
}

/**
 * @brief Chuyển đổi một số nguyên thành chuỗi hex in hoa, dạng little-endian.
 * * Ví dụ: 1234 (decimal) = 0x04D2 (hex)
 * Dạng little-endian: D204
 * * @param n Số nguyên dương để chuyển đổi.
 * @return std::string Chuỗi hex đã định dạng.
 */
std::string toLittleEndianHex(long long n) {
    std::stringstream ss;
    // Cấu hình stringstream để xuất hex, in hoa, và đệm bằng '0'
    ss << std::hex << std::uppercase << std::setfill('0');
    
    if (n == 0) {
        return "00";
    }

    long long temp = n;
    // Lấy từng byte (8 bit) từ LSB (byte thấp nhất) đến MSB (byte cao nhất)
    while (temp > 0) {
        // Lấy 8 bit cuối cùng (tương đương 1 byte)
        int byte = temp & 0xFF; 
        
        // Đặt chiều rộng là 2 và in byte đó (ví dụ: 5 -> "05")
        ss << std::setw(2) << byte;
        
        // Dịch phải 8 bit để lấy byte tiếp theo trong vòng lặp sau
        temp >>= 8;
    }
    
    return ss.str();
}

int main() {
    long long p, q, e;

    // Đọc 3 dòng đầu vào
    std::cin >> p >> q >> e;

    // 1. Tính phi(N) = (p - 1) * (q - 1)
    // Dùng __int128_t nếu p, q lớn, nhưng theo ràng buộc thư viện C++17 chuẩn
    // chúng ta giả định long long là đủ.
    long long phi = (p - 1) * (q - 1);

    // 2. Tìm d bằng Thuật toán Euclid Mở rộng
    long long d, k; // d và k là các giá trị x, y từ thuật toán
    long long gcd = extendedEuclidean(e, phi, d, k);

    // 3. Kiểm tra điều kiện
    // Nghịch đảo modular (d) chỉ tồn tại nếu e và phi là nguyên tố cùng nhau
    // tức là gcd(e, phi) == 1.
    if (gcd != 1) {
        std::cout << -1 << std::endl;
        return 0;
    }

    // 4. Tìm giá trị d dương nhỏ nhất
    // Kết quả 'd' từ EEA có thể là số âm.
    // Chúng ta cần tìm d' nhỏ nhất trong [1, phi-1] sao cho d' = d (mod phi).
    // Phép toán (d % phi + phi) % phi đảm bảo kết quả luôn là số dương.
    long long positive_d = (d % phi + phi) % phi;

    // 5. Chuyển đổi d sang dạng hex little-endian và in ra
    std::string hex_d = toLittleEndianHex(positive_d);
    
    std::cout << hex_d << std::endl;

    return 0;
}