#pragma once

#include <prelude.h>
#include <game/physics/engine.h>
#include <gdfe/input.h>

/* These bits represent what actions the client is requesting to take */
enum HumanoidActionBit {
    /* Movement */
    Jump = 0b1,
    Dash = 0b10,
    Sneak = 0b100,

    /* Combat */
};

DECL_NET_EVENT(HumanoidActionEvent) {
    u64 world_entity_id;

    // moving left and right
    // {-1, 0, 1}
    i8 x_input;
    // moving forward and back
    // {-1, 0, 1}
    i8 y_input;

    // A mask of HumanoidActionBit
    u64 mask = 0;

    FORCEINLINE void set_bits(u64 bits) {
        mask |= bits;
    }

    FORCEINLINE bool has_bits(u64 bits) const {
        return (mask & bits) != 0;
    }

    SERIALIZE_EVENT_FIELDS(
        x_input,
        y_input,
        mask
    );
};

struct MovementContext {
    ecs::Entity entity;

    bool dash_available = true;
    vec3 dash_dir;
    GDF_Stopwatch dash_stopwatch;

    MovementContext() {
        dash_stopwatch = GDF_StopwatchCreate();
    }

    ~MovementContext() {
        GDF_StopwatchDestroy(dash_stopwatch);
    }
};

namespace Components {
    /* This is specifically for the server. The client should have a diff
     * state machine.
     */
    struct MovementControl {
        explicit MovementControl(ecs::Entity entity);

        S(OnGround);
        S(InAir);
        S(Dashing);
        S(Falling);
        S(Jumping);

        using Ctx = hfsm2::Config::ContextT<MovementContext>;
        using M = hfsm2::MachineT<Ctx>;
        using FSM = M::PeerRoot<
            OnGround,
            M::Composite<
                InAir,
                Falling,
                Dashing,
                Jumping
            >
        >;

        /* Define states */
        struct OnGround : FSM::State {
            void enter(Control& control);
            void update(FullControl& control);
        };

        struct InAir : FSM::State {
            void enter(Control& control);
            void update(FullControl& control);
        };

        struct Dashing : FSM::State {
            void enter(Control& control);
            void update(FullControl& control);
            void exit(Control& control);
        };

        struct Jumping : FSM::State {
            void enter(Control& control);
            void update(FullControl& control);
            void exit(Control& control);
        };

        struct Falling : FSM::State {
            void enter(Control& control);
            void update(FullControl& control);
            void exit(Control& control);
        };

        FSM::Instance* state_machine;
    };
}