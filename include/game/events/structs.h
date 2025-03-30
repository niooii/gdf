#pragma once

#include <game/events.h>
#include <ser20/ser20.hpp>
#include <ser20/types/polymorphic.hpp>
#include <serde.h>

template<typename Derived>
struct EventBaseT : EventBase {
    void dispatch_self(EventManager& manager) const override {
        manager.dispatch(static_cast<const Derived&>(*this));
    }
};

#define DECL_EVENT(name, fields, ...) \
struct name : EventBaseT<name> { \
    fields \
    name() { type = EventTypeId::name; } \
    template<class Archive> \
    void serialize(Archive& ar) { \
        ar(ser20::base_class<EventBase>(this) __VA_OPT__(,) __VA_ARGS__); \
    } \
}; \
SER20_REGISTER_TYPE(name) \
SER20_REGISTER_POLYMORPHIC_RELATION(EventBase, name)

#define DEFINE_EVENT(name, fields, ...) DECL_EVENT(name, fields, ##__VA_ARGS__)
#include "defs.h"
#undef DEFINE_EVENT