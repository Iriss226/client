#pragma once

#include <cstdint>

inline int u64toa(std::uint64_t value, char* out) {
    char reversed[20];
    int length = 0;
    do {
        reversed[length++] = static_cast<char>('0' + value % 10);
        value /= 10;
    } while (value != 0);

    for (int i = 0; i < length; ++i) {
        out[i] = reversed[length - i - 1];
    }
    out[length] = '\0';
    return length;
}
