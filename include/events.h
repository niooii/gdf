#pragma once
#include <functional>
#include <gdfe/math/math.h>

enum class EventReplication {
	Local,
	ToServer,		// Client → Server
	ToClients,		// Server → Clients
	Bidirectional	// Client → Server → Client
};

enum class EventSource {
	Client,
	Server
};

struct EventBase {
	EventSource source = EventSource::Client;
	EventReplication replication = EventReplication::Local;
};

// These can be defined in individual files, or not idk
struct ChunkLoadEvent : EventBase {
	ivec3 chunk_coord;
};

template<typename EventT>
concept EventType = std::is_base_of_v<EventBase, EventT>;

template<EventType EventT>
class EventDispatcher {
	std::vector<std::function<void(const std::vector<EventT>&)>> handlers;
	std::vector<std::function<void(const EventT&)>> immediate_handlers;

	// should have a separate way to get realtime events, not just buffered
	// also use smart pointers instead of copying data
	std::vector<EventT> event_buffer = std::vector<EventT>(32);

public:
	// Subscribers will be notified when the event manager is flushed
	// Flushing will happen every frame after input is updated and
	// before rendering.
	void subscribe(std::function<void(const std::vector<EventT>&)> handler) {
		handlers.push_back(std::move(handler));
	}

	// Subscribers will be notified immediately when an event is dispatched.
	// Good for less frequently occurring events, or events that are
	// guarenteed to fire at most once a frame.
	// Using this for events that may happen more than once a frame
	// may drastically decrease performance.
	void subscribe_immediate(std::function<void(const EventT&)> handler) {
		immediate_handlers.push_back(std::move(handler));
	}

	void dispatch(const EventT& event) {
		if (!immediate_handlers.empty())
			for (auto& handler : immediate_handlers)
				handler(event);

		event_buffer.push_back(event);
	}

	// Should be called every frame.
	void flush() {
		for (auto& handler : handlers) {
			handler(event_buffer);
		}
		event_buffer.clear();
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
	void subscribe(std::function<void(const std::vector<T>&)> handler) {
		get_dispatcher<T>().subscribe(std::move(handler));
	}

	template<typename T>
	void subscribe_immediate(std::function<void(const T&)> handler) {
		get_dispatcher<T>().subscribe_immediate(std::move(handler));
	}

	// TODO! how to global flush lol gg
	// add global flush fn
	template<typename T>
	void flush() {
		get_dispatcher<T>().flush();
	}
};