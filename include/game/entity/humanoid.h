#pragma once

#include <prelude.h>
#include <game/physics/engine.h>
#include <gdfe/input.h>

struct MovementContext {
    ecs::Entity entity;
    Components::Velocity* vel;
    Components::AabbCollider* collider;

    bool dash_available = true;
};

namespace Components {
    struct Movement {
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
                Dashing,
                Falling,
                Jumping
            >
        >;

        /* Define states */
        struct OnGround : FSM::State {
            void update(FullControl& control) {
                if (GDF_IsKeyPressed(GDF_KEYCODE_Q) && control.context().dash_available)
                    control.changeTo<Dashing>();

                if (!control.context().collider->is_grounded)
                    control.changeTo<InAir>();
            }
        };

        struct InAir : FSM::State {
            void update(FullControl& control) {
                if (GDF_IsKeyPressed(GDF_KEYCODE_Q) && control.context().dash_available)
                    control.changeTo<Dashing>();
            }
        };

        struct Dashing : FSM::State {
            void enter(Control& control, MovementContext& context) {
                context.dash_available = false;
                vec3 forward, right, up;
                GDF_CameraOrientation(APP.main_camera, &forward, &right, &up);
                vec3 dash_vec = vec3_mul_scalar(forward, 1 * 40);
                context.vel->vec = dash_vec;
            }

            void exit(FullControl& control) {
                control.changeTo<Falling>();
            }
        };

        struct Jumping : FSM::State {
            void enter(Control& control, MovementContext& context) {
                vec3 up = GDF_CameraGetGlobalAxis(APP.main_camera);
                vec3_add_to(&context.vel->vec, vec3_mul_scalar(up, 1 * 40));
            }

            void update(FullControl& control) {
                if (control.context().vel->vec.y < -0.5)
                    control.changeTo<Falling>();
            }

            void exit(Control& control) {

            }
        };

        struct Falling : FSM::State {
            void enter(Control& control, MovementContext& context) {

            }

            void update(FullControl& control) {
                if (control.context().collider->is_grounded)
                    control.changeTo<OnGround>();
            }

            void exit(Control& control) {

            }
        };

        FSM::Instance state_machine;
    };
}