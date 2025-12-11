#include "input.hpp"
#include <cstring>
#include <cstdlib>

namespace catvim {

bool InputParser::parse(const std::string& input, Event& out, size_t& consumed) {
    if (input.empty()) {
        consumed = 0;
        return false;
    }
    
    unsigned char c = input[0];
    
    // Escape sequence or alt key
    if (c == 0x1b) {
        if (input.size() == 1) {
            // Just escape key
            out.type = EventType::KEY;
            out.key = {KEY_ESCAPE, false, false, false};
            consumed = 1;
            return true;
        }
        return parse_escape_sequence(input, out, consumed);
    }
    
    // Ctrl keys (1-26 except 9=tab, 13=enter)
    if (c >= 1 && c <= 26 && c != 9 && c != 13) {
        out.type = EventType::KEY;
        out.key = {'a' + c - 1, true, false, false};
        consumed = 1;
        return true;
    }
    
    // Regular character
    out.type = EventType::KEY;
    out.key = {c, false, false, false};
    consumed = 1;
    return true;
}

bool InputParser::parse_escape_sequence(const std::string& input, Event& out, size_t& consumed) {
    if (input.size() < 2) {
        consumed = 0;
        return false;
    }
    
    // Alt + key
    if (input[1] != '[' && input[1] != 'O') {
        out.type = EventType::KEY;
        out.key = {input[1], false, true, false};
        consumed = 2;
        return true;
    }
    
    // CSI sequence
    if (input[1] == '[') {
        return parse_csi_sequence(input, out, consumed);
    }
    
    // SS3 sequences (F1-F4 on some terminals)
    if (input[1] == 'O' && input.size() >= 3) {
        out.type = EventType::KEY;
        switch (input[2]) {
            case 'P': out.key = {KEY_F1, false, false, false}; break;
            case 'Q': out.key = {KEY_F2, false, false, false}; break;
            case 'R': out.key = {KEY_F3, false, false, false}; break;
            case 'S': out.key = {KEY_F4, false, false, false}; break;
            default:
                consumed = 1;
                out.key = {KEY_ESCAPE, false, false, false};
                return true;
        }
        consumed = 3;
        return true;
    }
    
    consumed = 1;
    out.type = EventType::KEY;
    out.key = {KEY_ESCAPE, false, false, false};
    return true;
}

bool InputParser::parse_csi_sequence(const std::string& input, Event& out, size_t& consumed) {
    // Looking for CSI sequences: \x1b[...
    if (input.size() < 3) {
        consumed = 0;
        return false;
    }
    
    // SGR mouse: \x1b[<Btn;X;Y[mM]
    if (input[2] == '<') {
        return parse_mouse_sgr(input, out, consumed);
    }
    
    // Find the end of CSI sequence
    size_t end = 2;
    while (end < input.size() && input[end] >= 0x30 && input[end] <= 0x3f) {
        end++;  // Parameter bytes
    }
    while (end < input.size() && input[end] >= 0x20 && input[end] <= 0x2f) {
        end++;  // Intermediate bytes
    }
    if (end >= input.size()) {
        consumed = 0;
        return false;
    }
    
    char final_byte = input[end];
    consumed = end + 1;
    
    out.type = EventType::KEY;
    out.key = {0, false, false, false};
    
    // Simple arrow keys and common sequences
    switch (final_byte) {
        case 'A': out.key.key = KEY_UP; break;
        case 'B': out.key.key = KEY_DOWN; break;
        case 'C': out.key.key = KEY_RIGHT; break;
        case 'D': out.key.key = KEY_LEFT; break;
        case 'H': out.key.key = KEY_HOME; break;
        case 'F': out.key.key = KEY_END; break;
        case '~': {
            // Parse number before ~
            std::string param(input.begin() + 2, input.begin() + end);
            int num = std::atoi(param.c_str());
            switch (num) {
                case 1: out.key.key = KEY_HOME; break;
                case 2: out.key.key = KEY_INSERT; break;
                case 3: out.key.key = KEY_DELETE; break;
                case 4: out.key.key = KEY_END; break;
                case 5: out.key.key = KEY_PAGE_UP; break;
                case 6: out.key.key = KEY_PAGE_DOWN; break;
                case 15: out.key.key = KEY_F5; break;
                case 17: out.key.key = KEY_F6; break;
                case 18: out.key.key = KEY_F7; break;
                case 19: out.key.key = KEY_F8; break;
                case 20: out.key.key = KEY_F9; break;
                case 21: out.key.key = KEY_F10; break;
                case 23: out.key.key = KEY_F11; break;
                case 24: out.key.key = KEY_F12; break;
                default: out.key.key = 0; break;
            }
            break;
        }
        default:
            out.key.key = 0;
            break;
    }
    
    return out.key.key != 0;
}

bool InputParser::parse_mouse_sgr(const std::string& input, Event& out, size_t& consumed) {
    // SGR format: \x1b[<Btn;X;Y[mM]
    // Find the terminator (m=release, M=press)
    size_t end = 3;
    while (end < input.size() && input[end] != 'm' && input[end] != 'M') {
        end++;
    }
    if (end >= input.size()) {
        consumed = 0;
        return false;
    }
    
    bool is_release = (input[end] == 'm');
    
    // Parse Btn;X;Y
    std::string params(input.begin() + 3, input.begin() + end);
    int btn = 0, x = 0, y = 0;
    
    size_t pos1 = params.find(';');
    if (pos1 == std::string::npos) {
        consumed = end + 1;
        return false;
    }
    size_t pos2 = params.find(';', pos1 + 1);
    if (pos2 == std::string::npos) {
        consumed = end + 1;
        return false;
    }
    
    btn = std::atoi(params.substr(0, pos1).c_str());
    x = std::atoi(params.substr(pos1 + 1, pos2 - pos1 - 1).c_str());
    y = std::atoi(params.substr(pos2 + 1).c_str());
    
    out.type = EventType::MOUSE;
    out.mouse.x = x;
    out.mouse.y = y;
    out.mouse.ctrl = (btn & 16) != 0;
    out.mouse.alt = (btn & 8) != 0;
    out.mouse.shift = (btn & 4) != 0;
    
    int button_code = btn & 3;
    bool is_drag = (btn & 32) != 0;
    bool is_scroll = (btn & 64) != 0;
    
    if (is_scroll) {
        out.mouse.action = MouseAction::SCROLL;
        out.mouse.button = (button_code == 0) ? MouseButton::SCROLL_UP : MouseButton::SCROLL_DOWN;
    } else if (is_drag) {
        out.mouse.action = MouseAction::DRAG;
        out.mouse.button = static_cast<MouseButton>(button_code);
    } else if (is_release) {
        out.mouse.action = MouseAction::RELEASE;
        out.mouse.button = static_cast<MouseButton>(button_code);
    } else {
        out.mouse.action = MouseAction::PRESS;
        out.mouse.button = static_cast<MouseButton>(button_code);
    }
    
    consumed = end + 1;
    return true;
}

}  // namespace catvim
