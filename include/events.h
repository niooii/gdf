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

class ChunkEventsListener {
	void onChunkLoad(const Chunk* chunk) {}
	void onChunkUpdate(const Chunk* chunk) {}
	void onChunkUnload(const Chunk* chunk) {}
}

// man wtf
class GlobalEventsManager {

public:
	void dispatchChunkUpdate()
};
