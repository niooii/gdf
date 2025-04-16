#pragma once
#include <gdfe/core.h>
#include <functional>
#include <memory>
#include <span>
#include <unordered_dense.h>
#include <ser20/ser20.hpp>
#include <ser20/archives/binary.hpp>
#include <spanstream>
#include <ser20/types/polymorphic.hpp>
#include <ser20/types/vector.hpp>
#include <constants.h>
#include <services/time.h>

/*
 * There is room for optimization here, but for now
 * forget about it
 * TODO! make events dispatch in SEQUENTIALLY IN QUEUED ORDER
 */

namespace Services::Events {
	struct NetEvent {
		virtual ~NetEvent() = default;

		ProgramType source = ProgramType::Client;
		// The unix timestamp of when the event was created from the source,
		// not deserialized. In UTC.
		u64 creation_time;

		// Intended for use when we store the base class only. Slightly slower
		virtual void dispatch() const = 0;

		// Intended for use when we store the base class only. Slightly slower
		virtual void queue_dispatch() const = 0;

		// Unused on the client. Do not set it, it will be ignored.
		std::string source_uuid;

		template<class Archive>
		void serialize(Archive& ar) {
			ar(source, creation_time);
		}
	};

	typedef u64 SubscriptionId;

	template<typename T>
	concept NetEventType = std::is_base_of_v<NetEvent, T>;

	struct Subscription {
		SubscriptionId id;
		virtual ~Subscription() = default;

		virtual void unsubscribe() const = 0;
	};

	namespace detail {

		template<typename>
		struct SubscriptionT : Subscription {
			~SubscriptionT() override = default;
			void unsubscribe() const override;
		};

		template<typename T>
		class EventDispatcher {
			// TODO! should separate deferred and immediate events. do this later
			// Deferred handlers
			ankerl::unordered_dense::map<SubscriptionId, std::function<void(const T&)>>
				handlers;

			// TODO! unused for now
			std::vector<std::function<bool(const T&)>> reject_conditions;

			// TODO! use smart pointers instead of copying data or something along those lines
			std::vector<T> event_buffer;

		public:
			EventDispatcher() {
				handlers.reserve(4);
				event_buffer.reserve(32);
			}

			// Subscribers will be notified when the event manager is flushed
			// Flushing will happen every frame after input is updated and
			// before rendering.
			std::unique_ptr<Subscription> subscribe(std::function<void(const T&)> handler) {
				static SubscriptionId next_id;

				SubscriptionId id = next_id++;
				handlers[id] = std::move(handler);

				LOG_INFO("added subscription. active handlers: %d", handlers.size());

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
			template<typename T>
			friend struct SubscriptionT;

			std::vector<std::function<void()>> flush_functions;

			template<typename T>
			static EventDispatcher<T>& get_dispatcher() {
				static EventDispatcher<T> dispatcher;
				static bool registered = false;

				if (!registered) {
					registered = true;
					get_instance().flush_functions.push_back([] {dispatcher.flush();});
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

			template<typename T>
			FORCEINLINE void dispatch(const T& event) {
				get_dispatcher<T>().dispatch(event);
			}

			template<typename T>
			FORCEINLINE void queue_dispatch(const T& event) {
				get_dispatcher<T>().queue_dispatch(event);
			}

			template<typename T>
			FORCEINLINE std::unique_ptr<Subscription> subscribe(std::function<void(const T&)> handler) {
				return get_dispatcher<T>().subscribe(std::move(handler));
			}

			template<typename T>
			// Add a condition to reject the dispatching of an event if true.
			// TODO! unused for now
			FORCEINLINE void reject_dispatch_if(std::function<bool(const T&)> reject_condition) {

			}

			FORCEINLINE void flush() {
				for (const auto& flush_fn : flush_functions) {
					flush_fn();
				}
			}
		};
	}

	// TODO can i attach these to event base instances maybe? idk.
	FORCEINLINE std::string serialize(const std::unique_ptr<NetEvent>& event) {
		std::ostringstream os;

		ser20::BinaryOutputArchive archive{os};
		archive(event);

		return os.str();
	}

	FORCEINLINE std::unique_ptr<NetEvent> deserialize(const std::span<char>& data) {
		std::unique_ptr<NetEvent> event;

		std::ispanstream stream{data};

		ser20::BinaryInputArchive archive{stream};
		archive(event);

		return event;
	}

	template<typename T>
	FORCEINLINE void dispatch(const T& event) {
		detail::EventManager::get_instance().dispatch(event);
	}

	template<typename T>
	FORCEINLINE void queue_dispatch(const T& event) {
		detail::EventManager::get_instance().queue_dispatch(event);
	}

	template<typename T>
	FORCEINLINE std::unique_ptr<Subscription> subscribe(std::function<void(const T&)> handler) {
		return detail::EventManager::get_instance().subscribe(handler);
	}

	template <typename T>
	void detail::SubscriptionT<T>::unsubscribe() const
	{
		detail::EventManager::get_dispatcher<T>().unsubscribe(id);
	}

	template<NetEventType T>
			FORCEINLINE std::unique_ptr<T> create_event() {
		T* e = new T();
		e->creation_time = Services::Time::unix_millis();
		e->source = CURR_PROGRAM_TYPE;
		return std::unique_ptr<T>(e);
	}

	template<NetEventType T, typename... Args>
	FORCEINLINE std::unique_ptr<T> create_event(Args&&... args) {
		std::unique_ptr<T> ptr = std::make_unique<T>(std::forward<Args>(args)...);
		ptr->creation_time = Services::Time::unix_millis();
		ptr->source = CURR_PROGRAM_TYPE;
		return std::move(ptr);
	}

	template<typename T>
	requires (!NetEventType<T>)
	FORCEINLINE std::unique_ptr<T> create_event() {
		return std::unique_ptr<T>(new T{});
	}

	template<typename T, typename... Args>
	requires (!NetEventType<T>)
	FORCEINLINE std::unique_ptr<T> create_event(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<NetEventType T>
	// Add a condition to reject the dispatching of an event if true.
	// TODO! unused for now
	FORCEINLINE void reject_dispatch_if(std::function<bool(const T&)> reject_condition) {

	}

	FORCEINLINE void flush() {
		detail::EventManager::get_instance().flush();
	}

	// Event template type
	template<typename Derived>
	struct NetEventT : NetEvent {
		void dispatch() const override {
			Events::dispatch(static_cast<const Derived&>(*this));
		}

		void queue_dispatch() const override {
			Events::queue_dispatch(static_cast<const Derived&>(*this));
		}
	};
}

SER20_REGISTER_TYPE(Services::Events::NetEvent)

