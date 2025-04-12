#pragma once
#include <game/types.h>
#include <gdfe/math/math.h>

// Helper macro for serializing fields on a type
#define SERIALIZE_FIELDS(...) \
template<class Archive> \
void serialize(Archive& ar) { \
ar(__VA_ARGS__); \
}

namespace ser20 {
    template <class Archive>
    void serialize(Archive& ar, vec3& v) {
        ar(v.x, v.y, v.z);
    }

    template <class Archive>
    void serialize(Archive& ar, ivec3& v) {
        ar(v.x, v.y, v.z);
    }

    template <class Archive>
    void serialize(Archive& ar, u8vec3& v) {
        ar(v.x, v.y, v.z);
    }
}
