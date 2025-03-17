#pragma once

#include <gdfe/core.h>
#include <gdfe/event.h>
#include <game/world.h>

struct EventBase {
    // some common properties
};

struct ChunkLoadEvent : EventBase {

};

struct ChunkUpdateEvent : EventBase {

};

struct ChunkUnloadEvent : EventBase {

};

class EventManager {
public:
    template<typename EventType>
    void registerListener(std::function<void(EventType&)> handler) {

    }

    template<typename EventType>
    void dispatchEvent(EventType& event) {

    }
};