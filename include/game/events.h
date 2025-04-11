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
#include <ser20/types/vector.hpp>

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

typedef u64 SubscriptionId;

template<typename T>
concept EventType = std::is_base_of_v<EventBase, T>;

struct Subscription {
	SubscriptionId id;

	virtual void unsubscribe() const = 0;
};

template<EventType>
struct SubscriptionT : Subscription {
	void unsubscribe() const override;
};

template<EventType T>
class EventDispatcher {
	// TODO! should separate deferred and immediate events. do this later
	// Deferred handlers
	ankerl::unordered_dense::map<SubscriptionId,std::function<void(const T&)>>
		handlers;

	// TODO! unused for now
	std::vector<std::function<bool(const T&)>> reject_conditions;

	// TODO! use smart pointers instead of copying data or something along those lines
	std::vector<T> event_buffer;

public:
	EventDispatcher() {
		event_buffer.reserve(32);
	}

	// Subscribers will be notified when the event manager is flushed
	// Flushing will happen every frame after input is updated and
	// before rendering.
	std::unique_ptr<Subscription> subscribe(std::function<void(const T&)> handler) {
		static SubscriptionId next_id;

		SubscriptionId id = next_id++;
		handlers[id] = std::move(handler);

		auto s = std::make_unique<SubscriptionT<T>>();
		s->id = id;

		return std::move(s);
	}

	void unsubscribe(SubscriptionId id)
	{
		handlers.erase(id);
	}

	void dispatch(const T& event) {
		if (!handlers.empty())
			for (auto& [_k, handler] : handlers)
				handler(event);
	}

	void queue_dispatch(const T& event) {
		event_buffer.push_back(event);
	}

	void flush() {
		for (auto& event : event_buffer) {
			for (auto& [_k, handler] : handlers) {
				handler(event);
			}
		}
		event_buffer.clear();
	}
};

class EventManager {
	template<EventType T>
	friend struct SubscriptionT;

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

	FORCEINLINE static EventManager& get_instance() {
		static EventManager instance;
		return instance;
	}

	// TODO can i attach these to event base instances maybe? idk.  
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
	FORCEINLINE void dispatch(const T& event) {
		get_dispatcher<T>().dispatch(event);
	}

	template<EventType T>
	FORCEINLINE void queue_dispatch(const T& event) {
		get_dispatcher<T>().queue_dispatch(event);
	}

	template<EventType T>
	FORCEINLINE std::unique_ptr<Subscription> subscribe(std::function<void(const T&)> handler) {
		return get_dispatcher<T>().subscribe(std::move(handler));
	}

	template<EventType T>
	FORCEINLINE std::unique_ptr<Subscription> subscribe_immediate(std::function<void(const T&)> handler) {
		return get_dispatcher<T>().subscribe_immediate(std::move(handler));
	}

	template<EventType T>
	FORCEINLINE std::unique_ptr<T> create_event() {
		return std::unique_ptr<T>(new T{});
	}

	template<EventType T, typename... Args>
	FORCEINLINE std::unique_ptr<T> create_event(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<EventType T>
	// Add a condition to reject the dispatching of an event if true.
	// TODO! unused for now
	FORCEINLINE void reject_dispatch_if(std::function<bool(const T&)> reject_condition) {

	}

	FORCEINLINE void flush() {
		for (auto flush_fn : flush_functions) {
			flush_fn();
		}
	}
};

template <EventType T>
void SubscriptionT<T>::unsubscribe() const
{
	EventManager::get_dispatcher<T>().unsubscribe(id);
}
