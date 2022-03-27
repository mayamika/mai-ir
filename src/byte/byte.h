#ifndef BYTE_H
#define BYTE_H

#include <iostream>
#include <string>
#include <vector>

namespace byte {

std::string read_string(std::istream& is) {
    int32_t n;
    is.read(reinterpret_cast<char*>(&n), sizeof(n));

    std::string s(n, ' ');
    is.read(reinterpret_cast<char*>(s.data()), n);

    return s;
}

int32_t write_string(std::ostream& os, const std::string& v) {
    int32_t n = v.size();
    os.write(reinterpret_cast<const char*>(&n), sizeof(n));
    os.write(reinterpret_cast<const char*>(v.data()), n);

    return n + sizeof(n);
}

template <typename T>
std::vector<T> read_vector(std::istream& is) {
    int32_t n;
    is.read(reinterpret_cast<char*>(&n), sizeof(n));

    std::vector<T> v(n);
    is.read(reinterpret_cast<char*>(v.data()), n * sizeof(T));

    return v;
}

template <typename T>
int32_t write_vector(std::ostream& os, const std::vector<T>& v) {
    int32_t n = v.size();
    os.write(reinterpret_cast<const char*>(&n), sizeof(n));

    int32_t bsize = n * sizeof(T);
    os.write(reinterpret_cast<const char*>(v.data()), bsize);

    return bsize + sizeof(n);
}

template <typename Int>
int32_t write_int(std::ostream& os, Int v) {
    static_assert(std::is_integral<Int>::value);
    os.write(reinterpret_cast<char*>(&v), sizeof(Int));

    return sizeof(Int);
}

template <typename Int>
Int read_int(std::istream& is) {
    static_assert(std::is_integral<Int>::value);

    Int v;
    is.read(reinterpret_cast<char*>(&v), sizeof(Int));

    return v;
}

}  // namespace byte

#endif
