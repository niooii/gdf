#pragma once
#include <vector>
#include <gdfe/def.h>
#include <memory>
#include <string>

class UIComponent;

struct UIScreen {
    std::vector<std::unique_ptr<UIComponent*>> components;
};

/// A UIComponent controls it's own rendering.
class UIComponent {
protected:
    virtual ~UIComponent() = default;

    u16 top = 0;
    u16 left = 0;

public:

    /// Checks input state and fires appropriate events. If a UI event is fired, then it is
    /// immediately dispatched, not deferred until a flush.
    virtual void handle_input() = 0;
};

class UIButtonComponent : UIComponent {
public:
    void handle_input() override;
};

class UITextBox : UIComponent {
    std::string text{32};
    bool focused = false;

public:
    std::string_view get_text();
    void set_text(const char* text);
    void set_text(std::string& text);

    void handle_input() override;
};

/// All instances of these events are guarenteed to be instantly dispatched.
struct UIButtonEvent {
    UIButtonComponent* button;
};

