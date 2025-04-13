#pragma once

#include <gdfe/core.h>
#include <game/ecs.h>
#include <game/hfsm.h>
#include <services/events.h>
#include <constants.h>

/* Macros for declaring event types */

// Helper macro for serializing fields on an event type
#define SERIALIZE_EVENT_FIELDS(...) \
template<class Archive> \
void serialize(Archive& ar) { \
ar(ser20::base_class<Event>(this) __VA_OPT__(,) __VA_ARGS__); \
}

// Declares a non-serializable event type. For serializable events, see DECL_SERDE_EVENT
#define DECL_EVENT(name) struct name : Services::Events::EventT<name>

// Every serializable event must contain SERIALIZE_FIELDS somewhere in it's declaration.
#define DECL_SERDE_EVENT(name) struct name; \
SER20_REGISTER_TYPE(name) \
SER20_REGISTER_POLYMORPHIC_RELATION(Services::Events::Event, name) \
struct name : Services::Events::EventT<name>