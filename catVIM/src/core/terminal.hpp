#pragma once

#include <string>
#include <termios.h>

namespace catvim {

struct Vec2 {
    int x, y;
};

class Terminal {
public:
    Terminal();
    ~Terminal();

    void enter_raw_mode();
    void exit_raw_mode();
    void enable_mouse();
    void disable_mouse();
    void enable_alternate_screen();
    void disable_alternate_screen();
    
    void write(const std::string& data);
    void write(const char* data, size_t len);
    void flush();
    
    int read_byte();  // Non-blocking single byte read
    std::string read_available();  // Non-blocking read all available
    bool poll_input(int timeout_ms);  // Wait for input with timeout
    
    Vec2 get_size();
    void clear();
    void move_cursor(int x, int y);
    void hide_cursor();
    void show_cursor();

private:
    struct termios original_termios_;
    bool raw_mode_enabled_ = false;
    bool mouse_enabled_ = false;
    bool alternate_screen_ = false;
};

}  // namespace catvim
