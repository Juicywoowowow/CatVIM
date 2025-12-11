#pragma once

#include <string>
#include <cstdint>

namespace catvim {

enum class EventType {
    NONE,
    KEY,
    MOUSE,
    RESIZE
};

enum class MouseButton {
    LEFT = 0,
    MIDDLE = 1,
    RIGHT = 2,
    RELEASE = 3,
    SCROLL_UP = 64,
    SCROLL_DOWN = 65
};

enum class MouseAction {
    PRESS,
    RELEASE,
    DRAG,
    SCROLL
};

struct KeyEvent {
    int key;           // ASCII or special key code
    bool ctrl = false;
    bool alt = false;
    bool shift = false;
};

struct MouseEvent {
    MouseAction action;
    MouseButton button;
    int x, y;          // 1-indexed
    bool ctrl = false;
    bool alt = false;
    bool shift = false;
};

struct Event {
    EventType type = EventType::NONE;
    union {
        KeyEvent key;
        MouseEvent mouse;
    };
    
    Event() : type(EventType::NONE) {}
};

// Special key codes (above ASCII range)
enum SpecialKey {
    KEY_ESCAPE = 27,
    KEY_ENTER = 13,
    KEY_TAB = 9,
    KEY_BACKSPACE = 127,
    KEY_UP = 256,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_INSERT,
    KEY_DELETE,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
    KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12
};

class InputParser {
public:
    // Parse input bytes into an event
    // Returns true if a complete event was parsed
    bool parse(const std::string& input, Event& out, size_t& consumed);

private:
    bool parse_escape_sequence(const std::string& input, Event& out, size_t& consumed);
    bool parse_mouse_sgr(const std::string& input, Event& out, size_t& consumed);
    bool parse_csi_sequence(const std::string& input, Event& out, size_t& consumed);
};

}  // namespace catvim
