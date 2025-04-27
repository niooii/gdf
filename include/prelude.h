#pragma once

#include <gdfe/core.h>
#include <game/ecs.h>
#include <game/hfsm.h>
#include <services/fwd.h>
#include <constants.h>
#include <serde.h>

/* Macros for declaring event types */

// Helper macro for serializing fields on an event type
#define SERIALIZE_EVENT_FIELDS(...) \
template<class Archive> \
void serialize(Archive& ar) { \
ar(ser20::base_class<NetEvent>(this) __VA_OPT__(,) __VA_ARGS__); \
}

// Every serializable event must contain SERIALIZE_FIELDS somewhere in it's declaration.
#define DECL_NET_EVENT(name) struct name; \
SER20_REGISTER_TYPE(name) \
SER20_REGISTER_POLYMORPHIC_RELATION(Services::Events::NetEvent, name) \
struct name : Services::Events::NetEventT<name>

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