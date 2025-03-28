#pragma once
#include <functional>
#include <gdfe/math/math.h>

/*
 * There is room for optimization here, but for now
 * forget about it
 */

enum class ProgramType {
	Client = 0,
	Server
};

enum class EventSendMode {
	// Calls local handlers
	Local = 0,
	// Client → Server
	Server,
	// Client → Server, then calls local handlers
	ServerAndLocal,
#ifdef GDF_SERVER_BUILD
	// Server → Clients, then calls server handlers.
	// Should only be specified on the server.
	ToClients,
#endif
};

class GlobalEventManager;

struct EventBase {
	ProgramType source = ProgramType::Client;
	EventSendMode replication = EventSendMode::Local;

	// Intended for use when we store the base class only. Slightly slower
	virtual void dispatch_self(GlobalEventManager& manager) const = 0;
};

template<typename EventT>
concept EventType = std::is_base_of_v<EventBase, EventT>;

template<EventType EventT>
class EventDispatcher {
	std::vector<std::function<void(const std::vector<EventT>&)>> handlers;
	std::vector<std::function<void(const EventT&)>> immediate_handlers;

	// TODO! use smart pointers instead of copying data or something along those lines
	std::vector<EventT> event_buffer;

public:
	EventDispatcher() {
		event_buffer.reserve(32);
	}

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

	void flush() {
		for (auto& handler : handlers) {
			handler(event_buffer);
		}
		event_buffer.clear();
	}
};

class GlobalEventManager {
	std::vector<void(*)()> flush_functions;

	template<typename T>
	static void flusher() {
		get_dispatcher<T>().flush();
	}

	template<typename T>
	static EventDispatcher<T>& get_dispatcher() {
		static EventDispatcher<T> dispatcher;
		static bool registered = false;

		if (!registered) {
			registered = true;
			get_instance().flush_functions.push_back(&flusher<T>);
		}

		return dispatcher;
	}

	GlobalEventManager() = default;

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

	void flush() {
		for (auto flush_fn : flush_functions) {
			flush_fn();
		}
	}
};

template<typename Derived>
struct EventBaseT : EventBase {
	void dispatch_self(GlobalEventManager& manager) const override {
		manager.dispatch(static_cast<const Derived&>(*this));
	}
};

// These can be defined in individual files, or not idk
struct ChunkLoadEvent : EventBaseT<ChunkLoadEvent> {
	ivec3 chunk_coord;
};

// These can be defined in individual files, or not idk
struct TestTextEvent : EventBaseT<TestTextEvent> {
	char* text;
};

struct ChunkUpdateEvent : EventBaseT<ChunkUpdateEvent> {
	ivec3 chunk_coord;
};
