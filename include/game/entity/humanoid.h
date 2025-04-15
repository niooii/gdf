#pragma once

#include <prelude.h>
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
    // The network synced entity ID that this action applies to.
    // This is ignored if a client sends it to a server, in which case
    // it will be processed for the player who sent it
    u64 world_entity_id;

    // The updated pitch of a humanoid - consider sending these separately?
    f32 pitch;
    // The updated yaw of a humanoid
    f32 yaw;

    // moving left and right
    // {-1, 0, 1}
    i8 x_input;
    // moving forward and back
    // {-1, 0, 1}
    i8 z_input;

    // A mask of HumanoidActionBit
    u64 mask = 0;

    FORCEINLINE void add_bits(u64 bits) {
        mask |= bits;
    }

    FORCEINLINE bool has_bits(u64 bits) const {
        return (mask & bits) != 0;
    }

    SERIALIZE_EVENT_FIELDS(
        x_input,
        z_input,
        mask
    );
};

namespace Systems {
    /* This is specifically for the server. The client should have a diff
     * state machine for effects, but have one for the main player for client side prediction.
     * This component depends on the Velocity, AabbCollider and Rotation components.
     */
    struct HumanoidMovementController {
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
        
        explicit HumanoidMovementController(ecs::Entity entity);
        ~HumanoidMovementController();

        S(OnGround);
        S(InAir);
        S(Dashing);
        S(Falling);
        S(Jumping);

        using Ctx = hfsm2::Config::ContextT<MovementContext>;
        using M = hfsm2::MachineT<Ctx>;
        using FSM = M::PeerRoot<
            OnGround,
            Dashing,
            M::Composite<
                InAir, // InAir is the parent of Falling, Jumping
                Falling,
                Jumping
            >
        >;

        /* Define states */
        struct OnGround : FSM::State {
            void enter(Control& control);
            void update(FullControl& control);
            void react(const HumanoidActionEvent& action, EventControl& control);
        };

        struct InAir : FSM::State {
            void enter(Control& control);
            void update(FullControl& control);
            void react(const HumanoidActionEvent& action, EventControl& control);
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

struct SimulatedHumanoid {
    ecs::Entity ecs_id;

    Systems::HumanoidMovementController movement_controller;

    SimulatedHumanoid(ecs::Entity ecs_id)
        : ecs_id{ecs_id}, movement_controller{ecs_id} {

    }
};