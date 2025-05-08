#pragma once
#include <constants.h>
#include <functional>
#include <gdfe/os/thread.h>
#include <gdfe/prelude.h>
#include <memory>
#include <ser20/archives/binary.hpp>
#include <ser20/ser20.hpp>
#include <ser20/types/polymorphic.hpp>
#include <ser20/types/vector.hpp>
#include <services/time.h>
#include <span>
#include <spanstream>
#include <unordered_dense.h>

/*
 * There is room for optimization here, but for now
 * forget about it
 * TODO! make events dispatch in SEQUENTIALLY IN QUEUED ORDER
 */

namespace Services::Events {
    typedef u64 SubscriptionId;

    struct Subscription {
        SubscriptionId id;
        virtual ~      Subscription() = default;

        virtual void unsubscribe() const = 0;
    };

    namespace detail {
        template <typename>
        struct SubscriptionT : Subscription {
            ~SubscriptionT() override = default;

            void unsubscribe() const override;
        };

        template <typename T>
        class EventDispatcher {
            ankerl::unordered_dense::map<SubscriptionId, std::function<void(const T&)>> handlers;

            // TODO! unused for now
            std::vector<std::function<bool(const T&)>> reject_conditions;

            std::vector<std::unique_ptr<T>> event_buffer;

        public:
            EventDispatcher()
            {
                handlers.reserve(4);
                event_buffer.reserve(32);
            }

            // Subscribers will be notified when the event manager is flushed
            // Flushing will happen every frame after input is updated and
            // before rendering.
            std::unique_ptr<Subscription> subscribe(std::function<void(const T&)> handler)
            {
                static SubscriptionId next_id;

                SubscriptionId id = next_id++;
                handlers[id]      = std::move(handler);

                LOG_INFO("added subscription. active handlers: %d", handlers.size());

                auto s = std::make_unique<SubscriptionT<T>>();
                s->id  = id;

                return std::move(s);
            }

            void unsubscribe(SubscriptionId id) { handlers.erase(id); }

            void dispatch(const T& event)
            {
                if (!handlers.empty())
                    for (auto& [_k, handler] : handlers)
                        handler(event);
            }

            void queue_dispatch(std::unique_ptr<T> event)
            {
                event_buffer.push_back(std::move(event));
            }

            void flush()
            {
                for (auto& event : event_buffer)
                {
                    for (auto& [_k, handler] : handlers)
                    {
                        handler(*event);
                    }
                }
                event_buffer.clear();
            }
        };

        class EventManager {
            template <typename T>
            friend struct SubscriptionT;

            std::vector<std::function<void()>> flush_functions;

            template <typename T>
            static EventDispatcher<T>& get_dispatcher()
            {
                static EventDispatcher<T> dispatcher;
                static bool               registered = false;

                if (!registered)
                {
                    registered = true;
                    get_instance().flush_functions.push_back([] { dispatcher.flush(); });
                }

                return dispatcher;
            }

            EventManager() = default;

        public:
                          EventManager(const EventManager&) = delete;
            EventManager& operator=(const EventManager&)    = delete;

            FORCEINLINE static EventManager& get_instance()
            {
                static EventManager instance;
                return instance;
            }

            template <typename T>
            FORCEINLINE void dispatch(const T& event)
            {
                get_dispatcher<T>().dispatch(event);
            }

            template <typename T>
            FORCEINLINE void queue_dispatch(std::unique_ptr<T> event)
            {
                get_dispatcher<T>().queue_dispatch(std::move(event));
            }

            template <typename T>
            FORCEINLINE std::unique_ptr<Subscription> subscribe(
                std::function<void(const T&)> handler)
            {
                return get_dispatcher<T>().subscribe(std::move(handler));
            }

            template <typename T>
            // Add a condition to reject the dispatching of an event if true.
            // TODO! unused for now
            FORCEINLINE void reject_dispatch_if(std::function<bool(const T&)> reject_condition)
            {}

            FORCEINLINE void flush()
            {
                for (const auto& flush_fn : flush_functions)
                    flush_fn();
            }
        };

        extern GDF_Semaphore flush_signal;
    } // namespace detail

    /// This returns after the next flush fully completes.
    /// This function is thread-safe.
    FORCEINLINE void wait_for_flush()
    {
        // consume the semaphore signal
        TODO("UNIMPLEMENTED")
    }

    template <typename T>
    FORCEINLINE void dispatch(const T& event)
    {
        detail::EventManager::get_instance().dispatch(event);
    }

    template <typename T>
    FORCEINLINE void queue_dispatch(std::unique_ptr<T> event)
    {
        detail::EventManager::get_instance().queue_dispatch(std::move(event));
    }

    template <typename T>
    FORCEINLINE std::unique_ptr<Subscription> subscribe(std::function<void(const T&)> handler)
    {
        return detail::EventManager::get_instance().subscribe(handler);
    }

    template <typename T>
    void detail::SubscriptionT<T>::unsubscribe() const
    {
        detail::EventManager::get_dispatcher<T>().unsubscribe(id);
    }

    template <typename T>
    FORCEINLINE std::unique_ptr<T> create_event()
    {
        return std::unique_ptr<T>(new T{});
    }

    template <typename T, typename... Args>
    FORCEINLINE std::unique_ptr<T> create_event(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    // Add a condition to reject the dispatching of an event if true.
    // TODO! unused for now
    // FORCEINLINE void reject_dispatch_if(std::function<bool(const T&)> reject_condition) {
    //
    // }

    FORCEINLINE void flush() { detail::EventManager::get_instance().flush(); }
} // namespace Services::Events
