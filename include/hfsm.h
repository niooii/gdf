#pragma once

class State {
// need a way to store children states. maybe templates will come in clutch
// or do i even need to store children?
protected:
	State* parent;

public:
	State(State* parent) : parent{parent} {
		
	};

    virtual void enter(Entity& e) {}
    virtual void exit(Entity& e) {}

    // User input should be handled here too, probably
    // Return a state to transition, or just return nullptr
    virtual State* update(Entity& e, f32 dt) {}

    // i lowkey want this to have a function that returns the state type,
    // so children can change behavior based on the parent state. but how?
    // wanan try to avoid runtmie overhead if possbile
};

// so how do i do this

// example models 

class AirborneState : State {
	
}

class JumpingState : AirborneState {
	
}

class FallingState : AirborneState {
	
}

class GroundedState : State {
public:
	void enter(Entity& e) override {
		
	}

	void exit(Entity& e) override {
		
	}

	State* update(Entity& e, f32 dt) override {
		if (GDF_IsKeyPressed(GDF_KEYCODE_SPACE)) {
			// now how do i set the parent state accordingly damn this is hard
			return new JumpingState();
		}
	}
}

// this could actually belong to an entity as a component - look more into that later	
class StateMachine {
	Entity e;
	State* curr;
	
public:
	StateMachine(State* entry, Entity e) : curr{entry}, e{e} {};
	// delete all states
	~StateMachine();

	// also consider an add_transition(std::function<State* transition(State* curr, f32 dt)>) where the fn
	// returns a new state or nothing. but calling a shit ton of functions doesnt seem that performant
	// however itll make things so much more clear
	// however thats basically already the update function man just ff
	void add_state(State* state);

	// something like this
	void update(f32 dt) {
		State* new_state = curr->update(e, dt);
		if (curr != new_state) {
			curr->exit(e);
			curr = new_state;
			curr->enter(e);
		}
	}
}
