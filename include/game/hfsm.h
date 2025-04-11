// #pragma once
// #include <game/events.h>
// #include <typeinfo>
//
// template <typename Derived, typename Parent = void>
// class State;
//
// template <typename InitialState>
// class StateMachine;
//
// template <typename T>
// struct has_parent : std::false_type {};
//
// template <typename D, typename P>
// struct has_parent<State<D, P>> : std::true_type {};
//
// template <typename Derived>
// class State<Derived, void> {
//
// public:
//     void enter() { static_cast<Derived*>(this)->on_enter(); }
//     void update(f32 dt) { static_cast<Derived*>(this)->on_update(dt); }
//     void exit() { static_cast<Derived*>(this)->on_exit(); }
//
//     template <typename StateT>
//     void transition() {
//         if (state_machine) {
//             static_cast<StateMachine*>(state_machine)->TransitionTo<StateT>();
//         }
//     }
//
//     void set_fsm_ptr(void* machine) {
//         state_machine = machine;
//     }
//
// protected:
//     void on_enter() {}
//     void on_update(f32 dt) {}
//     void on_exit() {}
//
//     void* state_machine = nullptr;
//
//     std::vector<std::unique_ptr<Subscription>> subscriptions;
//
//     template <EventType T>
//     void subscribe_event(std::function<void(const T&)> handler) {
//         auto& manager = EventManager::get_instance();
//         subscriptions.push_back(
//             std::move(
//                 manager.subscribe<T>(std::move(handler))
//             )
//         );
//     }
//
//     void unsubscribe_events() {
//         for (const auto& sub : subscriptions) {
//             sub->unsubscribe();
//         }
//         subscriptions.clear();
//     }
// };
//
// template <typename Derived, typename Parent>
// class State<Derived, Parent> : public Parent {
//
// public:
//     void enter() {
//         Parent::enter();
//         static_cast<Derived*>(this)->on_enter();
//     }
//
//     void update(float dt) {
//         Parent::update(dt);
//         static_cast<Derived*>(this)->on_update(dt);
//     }
//
//     void exit() {
//         static_cast<Derived*>(this)->on_exit();
//         Parent::exit();
//     }
//
// protected:
//     void on_enter() {}
//     void on_update(f32 dt) {}
//     void on_exit() {}
// };
//
// template <typename InitialState>
// class StateMachine {
// public:
//     StateMachine() {
//         current_state_ptr = &initial_state;
//         current_state_ptr->set_fsm_ptr(this);
//         current_state_ptr->enter();
//     }
//
//     ~StateMachine() {
//         if (current_state_ptr) {
//             current_state_ptr->exit();
//         }
//     }
//
//     void update(float dt) {
//         if (current_state_ptr) {
//             current_state_ptr->update(dt);
//         }
//     }
//
//     template <typename NewStateT>
//     void transition() {
//         if (current_state_ptr) {
//             current_state_ptr->exit();
//             current_state_ptr->unsubscribe_events();
//         }
//
//         // TODO!
//
//     }
//
//     // Check if in a specific state or its parent states
//     // TODO!
//     template <typename StateT>
//     bool is_in_state() const {
//         return false;
//     }
//
// private:
//     InitialState initial_state;
//
//     State<InitialState, void>* current_state_ptr = nullptr;
// };