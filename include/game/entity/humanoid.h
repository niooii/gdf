#pragma once

#include <gdfe/input.h>
#include <prelude.h>

/* These bits represent what actions the client is requesting to take */
enum HumanoidActionBit {
    /* Movement */
    Jump  = 0b1,
    Dash  = 0b10,
    Sneak = 0b100,

    /* General */
    Attack = 0b1000,
    Use    = 0b10000,
};

DECL_PACKET(HumanoidStateChangeEvent)
{
    /* Shared usage by client and server */
    // The updated pitch of a humanoid - consider sending these separately?
    f32 pitch = 0;
    // The updated yaw of a humanoid
    f32 yaw = 0;

    // moving left and right
    // {-1, 0, 1}
    i8 x_input;
    // moving forward and back
    // {-1, 0, 1}
    i8 z_input;

    // A mask of HumanoidActionBit
    BitField action_bits{};

    /*
     * The following fields are unused when processing a state change request
     * from the client. The server will ignore them.
     * They are intended for use when the server send humanoid state changes to the client.
     * (or consider sending position updates separately, but world_entity_id is still needed)
     */

    // The network synced entity ID that this action applies to.
    // This is ignored if a client sends it to a server, in which case
    // it will be processed for the player who sent it
    u64 world_entity_id;
    // Intended use - physics extrapolation
    // represents the bottom left corner (or min field) of the entity's aabb
    vec3 pos;
    // Intended use - physics extrapolation
    vec3 vel;

    FORCEINLINE bool has_bits(u64 bits) const { return action_bits.has_bits(bits); }
    FORCEINLINE void add_bits(u64 bits) { return action_bits.add_bits(bits); }

    SERIALIZE_PACKET_FIELDS(x_input, z_input, pitch, yaw, action_bits, world_entity_id, pos, vel);
};

namespace Systems {
    struct MovementContext {
        ecs::Entity entity;

        bool          dash_available = true;
        vec3          dash_dir;
        GDF_Stopwatch dash_stopwatch;

        // moving left and right
        // {-1, 0, 1}
        i8 x_input = 0;
        // moving forward and back
        // {-1, 0, 1}
        i8 z_input = 0;

        // A mask of HumanoidActionBit
        BitField action_bits;

        HumanoidStateChangeEvent* accumulator = NULL;

        MovementContext() { dash_stopwatch = GDF_StopwatchCreate(); }

        ~MovementContext() { GDF_StopwatchDestroy(dash_stopwatch); }

        // Updates the shared movement context with the most recent humanoid action event
        FORCEINLINE void set_most_recent_action(const HumanoidStateChangeEvent& action)
        {
            x_input     = action.x_input;
            z_input     = action.z_input;
            action_bits = action.action_bits;
        }
    };
    /* This is specifically for the server. The client should have a diff
     * state machine for effects, but have one for the main player for client side prediction.
     * This component depends on the Velocity, AabbCollider and Rotation components.
     */
    struct HumanoidMovementController {
        explicit HumanoidMovementController(ecs::Entity entity);
        ~        HumanoidMovementController();

        S(OnGround);
        S(InAir);
        S(Dashing);
        S(Falling);
        S(Jumping);

        using Ctx = hfsm2::Config::ContextT<MovementContext*>;
        using M   = hfsm2::MachineT<Ctx>;
        using FSM = M::PeerRoot<OnGround, Dashing,
            M::Composite<InAir, // InAir is the parent of Falling, Jumping
                Falling, Jumping>>;

        /* Define states */
        struct OnGround : FSM::State {
            void enter(Control& control);
            void update(FullControl& control);
            void react(const HumanoidStateChangeEvent& action, EventControl& control);
        };

        struct InAir : FSM::State {
            void enter(Control& control);
            void update(FullControl& control);
            void react(const HumanoidStateChangeEvent& action, EventControl& control);
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

        // Sets the action event to be modified as states are processed.
        FORCEINLINE void set_action_accumulator(HumanoidStateChangeEvent* action_accumulator) const
        {
            ctx->accumulator = action_accumulator;
        }

        FORCEINLINE void process_action(const HumanoidStateChangeEvent& action) const
        {
            ctx->set_most_recent_action(action);

            // These should always be valid - we shouldn't restrict their pitch and yaw.. or maybe?
            ctx->accumulator->pitch = action.pitch;
            ctx->accumulator->yaw   = action.yaw;

            state_machine->react(action);
        }

        FORCEINLINE void update() const { state_machine->update(); }

        MovementContext* ctx;
        FSM::Instance*   state_machine;
    };
} // namespace Systems

class SimulatedHumanoid {
    ecs::Entity ecs_id;

    Systems::HumanoidMovementController movement_controller;

    // state machiens should rebuild this event based on what actually happened -
    // that way, we only send the valid events back to clients
    HumanoidStateChangeEvent action_accumulator{};

public:
    SimulatedHumanoid(ecs::Entity ecs_id) : ecs_id{ ecs_id }, movement_controller{ ecs_id }
    {

        movement_controller.set_action_accumulator(&action_accumulator);
    }

    /// Resets the accumulated actions of the humanoid
    FORCEINLINE void reset_accumulator()
    {
        GDF_Memset(&action_accumulator, 0, sizeof(action_accumulator));
    }

    /// Get the accumulated actions the humanoid has taken. Should be called after the update()
    /// function. This will contain only validated actions (actions the humanoid has TAKEN, not
    /// requested), so it's good to send back to the client
    FORCEINLINE HumanoidStateChangeEvent& accumulated_actions() { return action_accumulator; }

    FORCEINLINE void process_action(const HumanoidStateChangeEvent& action) const
    {
        movement_controller.process_action(action);
    }

    FORCEINLINE void update() { movement_controller.update(); }
};
