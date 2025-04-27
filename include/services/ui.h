#pragma once

#include <game/ui.h>

// In the global services namespace for now in case the server will have a UI later.
namespace Services::UI {
    UIScreen* top_screen();
    UIScreen* pop_screen();
    void pop_all_screens();
    UIScreen* push_screen(UIScreen* );
}
