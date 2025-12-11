#include "terminal.hpp"
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <cstdio>
#include <cstring>

namespace catvim {

Terminal::Terminal() {}

Terminal::~Terminal() {
    if (mouse_enabled_) disable_mouse();
    if (alternate_screen_) disable_alternate_screen();
    if (raw_mode_enabled_) exit_raw_mode();
}

void Terminal::enter_raw_mode() {
    if (raw_mode_enabled_) return;
    
    tcgetattr(STDIN_FILENO, &original_termios_);
    struct termios raw = original_termios_;
    
    // Input flags: no break, no CR to NL, no parity check, no strip, no flow ctrl
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // Output flags: disable post processing
    raw.c_oflag &= ~(OPOST);
    // Control flags: set 8-bit chars
    raw.c_cflag |= (CS8);
    // Local flags: no echo, no canonical, no extended, no signals
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    // Non-blocking: return immediately with whatever is available
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;  // No timeout - non-blocking
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    raw_mode_enabled_ = true;
}

void Terminal::exit_raw_mode() {
    if (!raw_mode_enabled_) return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios_);
    raw_mode_enabled_ = false;
}

void Terminal::enable_mouse() {
    if (mouse_enabled_) return;
    // Enable mouse tracking: button events + SGR extended mode
    write("\x1b[?1000h");  // Enable mouse button tracking
    write("\x1b[?1002h");  // Enable button event tracking (drag)
    write("\x1b[?1006h");  // Enable SGR extended mode
    flush();
    mouse_enabled_ = true;
}

void Terminal::disable_mouse() {
    if (!mouse_enabled_) return;
    write("\x1b[?1006l");
    write("\x1b[?1002l");
    write("\x1b[?1000l");
    flush();
    mouse_enabled_ = false;
}

void Terminal::enable_alternate_screen() {
    if (alternate_screen_) return;
    write("\x1b[?1049h");
    flush();
    alternate_screen_ = true;
}

void Terminal::disable_alternate_screen() {
    if (!alternate_screen_) return;
    write("\x1b[?1049l");
    flush();
    alternate_screen_ = false;
}

void Terminal::write(const std::string& data) {
    ::write(STDOUT_FILENO, data.c_str(), data.size());
}

void Terminal::write(const char* data, size_t len) {
    ::write(STDOUT_FILENO, data, len);
}

void Terminal::flush() {
    fsync(STDOUT_FILENO);
}

int Terminal::read_byte() {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1) {
        return static_cast<unsigned char>(c);
    }
    return -1;
}

std::string Terminal::read_available() {
    std::string result;
    char buf[128];
    ssize_t n;
    while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        result.append(buf, n);
    }
    return result;
}

Vec2 Terminal::get_size() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return {80, 24};  // Default fallback
    }
    return {ws.ws_col, ws.ws_row};
}

void Terminal::clear() {
    write("\x1b[2J");
}

void Terminal::move_cursor(int x, int y) {
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", y, x);
    write(buf);
}

void Terminal::hide_cursor() {
    write("\x1b[?25l");
}

void Terminal::show_cursor() {
    write("\x1b[?25h");
}

bool Terminal::poll_input(int timeout_ms) {
    struct pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    return poll(&pfd, 1, timeout_ms) > 0;
}

}  // namespace catvim
