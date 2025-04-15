#pragma once

#include <gdfe/core.h>
#include <game/ecs.h>
#include <game/hfsm.h>
#include <services/events.h>
#include <services/global.h>
#include <constants.h>

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