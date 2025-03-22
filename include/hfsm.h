#pragma once

class State {
public:
    virtual void enter() {}
    virtual void exit() {}

    // User input should be handled here too, probably
    virtual void update(f32 dt) {}
};

// so how do i do this