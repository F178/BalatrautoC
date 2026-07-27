#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <limits>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>

#if defined(_MSC_VER)
#define INLINE_FORCE __forceinline
#else
#define INLINE_FORCE __attribute__((always_inline)) inline
#endif

constexpr uint64_t MAX_UINT64 = 18446744073709551615ull;

typedef union DoubleLong {
    double dbl;
    uint64_t ulong;
} dbllong;

struct LuaRandom {
    uint64_t state[4];
    LuaRandom(double seed) {
        double d = seed;
        uint64_t r = 0x11090601;
        for (int i = 0; i < 4; i++) {
            uint64_t m = 1ull << (r & 255);
            r >>= 8;
            d = d * 3.14159265358979323846 + 2.7182818284590452354;
            dbllong u;
            u.dbl = d;
            if (u.ulong < m) u.ulong += m;
            state[i] = u.ulong;
        }
        for (int i = 0; i < 10; i++) {
            _randint();
        }
    }
    LuaRandom() {
        LuaRandom(0);
    }
    uint64_t _randint() {
        uint64_t z = 0;
        uint64_t r = 0;
        z = state[0];
        z = (((z<<31ull)^z)>>45ull)^((z&(MAX_UINT64<<1ull))<<18ull);
        r ^= z;
        state[0] = z;
        z = state[1];
        z = (((z<<19ull)^z)>>30ull)^((z&(MAX_UINT64<<6ull))<<28ull);
        r ^= z;
        state[1] = z;
        z = state[2];
        z = (((z<<24ull)^z)>>48ull)^((z&(MAX_UINT64<<9ull))<<7ull);
        r ^= z;
        state[2] = z;
        z = state[3];
        z = (((z<<21ull)^z)>>39ull)^((z&(MAX_UINT64<<17ull))<<8ull);
        r ^= z;
        state[3] = z;
        return r;
    }
    uint64_t randdblmem() {        
        return (_randint() & 4503599627370495ull) | 4607182418800017408ull;
    }
    double random() {
        dbllong u;
        u.ulong = randdblmem();
        return u.dbl - 1.0;
    }
    int randint(int min, int max) {
        return (int)(random()*(max-min+1))+min;
    }
};

INLINE_FORCE double pseudohash(const std::string& s) {
    static constexpr double MAGIC1 = 1.1239285023;
    static constexpr double PI = 3.141592653589793116;
    
    double num = 1.0;
    const size_t len = s.length();
    const char* data = s.data(); // Avoid bounds checking in loop
    
    for (size_t i = 0; i < len; i++) {
        // Inline division (still exact)
        double temp = MAGIC1 / num * data[len-1-i] * PI + PI * (len-i);
        num = temp - std::floor(temp);
    }
    
    return std::isnan(num) ? std::numeric_limits<double>::quiet_NaN() : num;
};

constexpr double inv_prec = 10000000000000.0;  // std::pow(10.0, 13)
constexpr double two_inv_prec = 8192.0;         // std::pow(2.0, 13)
constexpr double five_inv_prec = 1220703125.0;  // std::pow(5.0, 13)

INLINE_FORCE double round13(double x) {
    double tentative = std::floor(x * inv_prec) / inv_prec;
    // Optimize fmod(x, 1.0) = x - floor(x)
    double temp1 = x * two_inv_prec;
    double truncated = (temp1 - std::floor(temp1)) * five_inv_prec;
    double temp2 = truncated;
    if (tentative != x && tentative != std::nextafter(x, 1) && (temp2 - std::floor(temp2)) >= 0.5) {
        return (std::floor(x * inv_prec) + 1) / inv_prec;
    }
    return tentative;
}
