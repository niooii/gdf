#pragma once
#include <functional>
#include <gdfe/math/math.h>

struct EventBase {
	enum class Source { Client, Server } source = Source::Client;
	enum class ReplicationType {
		Local,
		ToServer,		// Client → Server
		ToClients,		// Server → Clients
		Bidirectional	// Client → Server → Client
	} replication = ReplicationType::Local;
};

// These can be defined in individual files, or not idk
struct ChunkLoadEvent : EventBase {
	ivec3 chunk_coord;
};

template<typename EventT>
concept EventType = std::is_base_of_v<EventBase, EventT>;

template<EventType EventT>
class EventDispatcher {
	std::vector<std::function<void(const EventT&)>> handlers;

public:
	void subscribe(std::function<void(const EventT&)> handler) {
		handlers.push_back(std::move(handler));
	}

	// TODO! add a nice way to queue these events until each frame ends then flush all at once
	void dispatch(const EventT& event) {
		for (auto& handler : handlers) {
			handler(event);
		}
	}
};

class GlobalEventManager {
	GlobalEventManager() = default;

	template<typename T>
	static EventDispatcher<T>& get_dispatcher() {
		static EventDispatcher<T> dispatcher;
		return dispatcher;
	}

public:
	GlobalEventManager(const GlobalEventManager&) = delete;
	GlobalEventManager& operator=(const GlobalEventManager&) = delete;

	static GlobalEventManager& get_instance() {
		static GlobalEventManager instance;
		return instance;
	}

	template<typename T>
	void dispatch(const T& event) {
		get_dispatcher<T>().dispatch(event);
	}

	template<typename T>
	void subscribe(std::function<void(const T&)> handler) {
		get_dispatcher<T>().subscribe(std::move(handler));
	}
};