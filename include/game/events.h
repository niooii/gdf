#pragma once
#include <functional>
#include <memory>
#include <span>
#include <unordered_dense.h>
#include <game/events/type_ids.h>
#include <ser20/ser20.hpp>
#include <ser20/archives/binary.hpp>
#include <spanstream>
#include <ser20/types/polymorphic.hpp>

/*
 * There is room for optimization here, but for now
 * forget about it
 */

enum class ProgramType {
	Client = 0,
	Server
};

// enum class DispatchMode {
// 	// Calls local handlers only
// 	Local = 0,
// 	// Client → Server only, no local handlers are called
// 	PureReplicated,
// 	// Client → Server, then calls local handlers
// 	Replicated,
// // #ifdef GDF_SERVER_BUILD
// // 	// Server → Clients, then calls server handlers.
// // 	// Should only be specified on the server.
// // 	ToClients,
// // #endif
// };



class EventManager;

struct EventBase {
	virtual ~EventBase() = default;

	ProgramType source = ProgramType::Client;
	// DispatchMode replication = DispatchMode::Local;

	// EventTypeId type;

	// Intended for use when we store the base class only. Slightly slower
	virtual void dispatch_self(EventManager& manager) const = 0;

	template<class Archive>
	void serialize(Archive& ar) {
		ar(source/*, replication*/);
	}
};

SER20_REGISTER_TYPE(EventBase)

template<typename EventT>
concept EventType = std::is_base_of_v<EventBase, EventT>;

template<EventType EventT>
class EventDispatcher {
	// TODO! should separate deferred and immediate events. do this later
	// Deferred handlers
	std::vector<std::function<void(const std::vector<EventT>&)>> handlers;
	// Instant handlers, called immediately on dispatch
	std::vector<std::function<void(const EventT&)>> immediate_handlers;

	// TODO! unused for now
	std::vector<std::function<bool(const EventT&)>> reject_conditions;

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

class EventManager {
	std::vector<void(*)()> flush_functions;

	// For generating polymorphic EventBase instances
	// TODO! trhiws is actually not needed
	using GenerateEventBase = std::function<std::unique_ptr<EventBase>()>;
	GenerateEventBase generators[(u32)EventTypeId::MaxEvent]{};

	template<EventType T>
	static void flusher() {
		get_dispatcher<T>().flush();
	}

	template<EventType EventT>
	static EventDispatcher<EventT>& get_dispatcher() {
		static EventDispatcher<EventT> dispatcher;
		static bool registered = false;

		if (!registered) {
			registered = true;
			get_instance().flush_functions.push_back(&flusher<EventT>);

			// TODO! this stupid hack
			// EventT tmp = EventT{};
			// const u32 id = (u32)(tmp.type);
			// get_instance().generators[id] = [] { return std::make_unique<EventT>(); };
		}

		return dispatcher;
	}

	EventManager() = default;

public:
	EventManager(const EventManager&) = delete;
	EventManager& operator=(const EventManager&) = delete;

	static EventManager& get_instance() {
		static EventManager instance;
		return instance;
	}

	std::unique_ptr<EventBase> create_event(EventTypeId type_id) {
		if (auto generator = generators[(u32)type_id]) {
			return generator();
		}
		return nullptr;
	}

	std::string serialize(const std::unique_ptr<EventBase>& event) {
		std::ostringstream os;

		ser20::BinaryOutputArchive archive{os};
		archive(event);

		return os.str();
	}

	std::unique_ptr<EventBase> deserialize(const std::span<char>& data) {
		std::unique_ptr<EventBase> event;

		std::ispanstream stream{data};

		ser20::BinaryInputArchive archive{stream};
		archive(event);

		return event;
	}

	template<EventType T>
	void dispatch(const T& event) {
		get_dispatcher<T>().dispatch(event);
	}

	template<EventType T>
	void subscribe(std::function<void(const std::vector<T>&)> handler) {
		get_dispatcher<T>().subscribe(std::move(handler));
	}

	template<EventType T>
	void subscribe_immediate(std::function<void(const T&)> handler) {
		get_dispatcher<T>().subscribe_immediate(std::move(handler));
	}

	template<EventType T>
	// Add a condition to reject the dispatching of an event if true.
	// TODO! unused for now
	void reject_dispatch_if(std::function<bool(const T&)> reject_condition) {

	}

	void flush() {
		for (auto flush_fn : flush_functions) {
			flush_fn();
		}
	}
};

