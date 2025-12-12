#include <cstdint> 

struct MagicRNG {
    uint32_t state = 1804289383; 

    static MagicRNG& get() {
        static MagicRNG instance;
        return instance;
    }

    uint32_t random_32() {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;
        return x;
    }

    uint64_t random_64() {
        uint64_t u1 = (uint64_t)(random_32()) & 0xFFFF;
        uint64_t u2 = (uint64_t)(random_32()) & 0xFFFF;
        uint64_t u3 = (uint64_t)(random_32()) & 0xFFFF;
        uint64_t u4 = (uint64_t)(random_32()) & 0xFFFF;
        
        return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
    }

    uint64_t random_sparse() {
        return random_64() & random_64() & random_64();
    }
};