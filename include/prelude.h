#pragma once

#include <gdfe/prelude.h>
#include <game/ecs.h>
#include <game/hfsm.h>
#include <game/net.h>
#include <services/fwd.h>
#include <constants.h>
#include <serde.h>

/* Utility stuff */
struct BitField {
    u64 mask;

    BitField()
        : mask{0} {}

    FORCEINLINE void add_bits(const u64 bits) {
        mask |= bits;
    }

    FORCEINLINE bool has_bits(const u64 bits) const {
        return (mask & bits) != 0;
    }

    SERIALIZE_FIELDS(mask);
};