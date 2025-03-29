#pragma once

#include "type_ids.h"
#include <game/events.h>

template<typename Derived>
struct EventBaseT : EventBase {
    void dispatch_self(EventManager& manager) const override {
        manager.dispatch(static_cast<const Derived&>(*this));
    }
};

#define DECL_EVENT(name, fields) \
struct name : EventBaseT<name> { \
    fields \
    name() { type = EventTypeId::name; } \
};

#define DEFINE_EVENT(name, fields) DECL_EVENT(name, fields)
#include "defs.h"
