#pragma once
#include "unordered_dense.h"
#include <gdfe/math/math.h>

// Hash implementations

template <>
struct ankerl::unordered_dense::hash<ivec3> {
    using is_avalanching = void;

    [[nodiscard]] auto operator()(ivec3 const& x) const noexcept -> uint64_t {
        // primes n stuff
        const u32 p1 = 73856093u;
        const u32 p2 = 19349663u;
        const u32 p3 = 83492791u;

        u32 h1 = (u32)(x.x) * p1;
        u32 h2 = (u32)(x.y) * p2;
        u32 h3 = (u32)(x.z) * p3;

        return h1 ^ h2 ^ h3;
    }
};

// Operator overloads

inline bool operator==(const ivec3& a, const ivec3& b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}
