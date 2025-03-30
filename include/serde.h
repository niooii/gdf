#pragma once
#include <gdfe/math/math.h>

namespace ser20 {
    template <class Archive>
    void serialize(Archive& ar, vec3& v) {
        ar(v.x, v.y, v.z);
    }

    template <class Archive>
    void serialize(Archive& ar, ivec3& v) {
        ar(v.x, v.y, v.z);
    }
}
