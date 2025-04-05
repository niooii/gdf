#pragma once

#include <game/events.h>
#include <ser20/ser20.hpp>
#include <ser20/types/polymorphic.hpp>
#include <serde.h>
#include <vector>
#include <game/types.h>
#include <game/world.h>

#define FIELDS(...) __VA_ARGS__

template<typename Derived>
struct EventBaseT : EventBase {
    void dispatch_self(EventManager& manager) const override {
        manager.dispatch(static_cast<const Derived&>(*this));
    }
};

#define SERIALIZE_FIELDS(...) \
    template<class Archive> \
    void serialize(Archive& ar) { \
        ar(ser20::base_class<EventBase>(this) __VA_OPT__(,) __VA_ARGS__); \
    }
#define MAKE_SERIALIZABLE(struct_name) \
SER20_REGISTER_TYPE(struct_name) \
SER20_REGISTER_POLYMORPHIC_RELATION(EventBase, struct_name)

#define DECL_EVENT(name) struct name : EventBaseT<name>

DECL_EVENT(ChunkLoadEvent)
{
    ivec3 chunk_coord;
    SERIALIZE_FIELDS(chunk_coord)
};
MAKE_SERIALIZABLE(ChunkLoadEvent)

DECL_EVENT(TestTextEvent)
{
    std::string message;
    SERIALIZE_FIELDS(message)
};
MAKE_SERIALIZABLE(TestTextEvent)

DECL_EVENT(ChunkUpdateEvent)
{
    ivec3 chunk_coord;
    std::vector<std::pair<u8vec3, u8vec3>> updated;
    SERIALIZE_FIELDS(chunk_coord)
};
MAKE_SERIALIZABLE(ChunkUpdateEvent)

DECL_EVENT(PlayerMoveEvent)
{
    vec3 pos;
    SERIALIZE_FIELDS(pos)
};
MAKE_SERIALIZABLE(PlayerMoveEvent)