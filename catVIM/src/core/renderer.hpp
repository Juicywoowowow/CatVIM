#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace catvim {

struct Color {
    uint8_t r, g, b;
    bool is_default = true;
    
    static Color Default() { return {0, 0, 0, true}; }
    static Color RGB(uint8_t r, uint8_t g, uint8_t b) { return {r, g, b, false}; }
    static Color Index(int idx) {
        // Basic 16 colors mapping
        return {static_cast<uint8_t>(idx), 0, 0, false}; 
    }
};

enum class Attr : uint8_t {
    NONE = 0,
    BOLD = 1 << 0,
    DIM = 1 << 1,
    ITALIC = 1 << 2,
    UNDERLINE = 1 << 3,
    BLINK = 1 << 4,
    REVERSE = 1 << 5,
    STRIKETHROUGH = 1 << 6
};

inline Attr operator|(Attr a, Attr b) {
    return static_cast<Attr>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline bool operator&(Attr a, Attr b) {
    return (static_cast<uint8_t>(a) & static_cast<uint8_t>(b)) != 0;
}

struct Style {
    Color fg = Color::Default();
    Color bg = Color::Default();
    Attr attrs = Attr::NONE;
    
    bool operator==(const Style& other) const;
    bool operator!=(const Style& other) const { return !(*this == other); }
};

struct Cell {
    char32_t ch = ' ';
    Style style;
    
    bool operator==(const Cell& other) const {
        return ch == other.ch && style == other.style;
    }
    bool operator!=(const Cell& other) const { return !(*this == other); }
};

class Renderer {
public:
    Renderer();
    
    void resize(int width, int height);
    int width() const { return width_; }
    int height() const { return height_; }
    
    void set_cell(int x, int y, char32_t ch, const Style& style);
    void set_cell(int x, int y, char32_t ch);
    void set_string(int x, int y, const std::string& str, const Style& style);
    void set_style(const Style& style) { current_style_ = style; }
    
    void clear();
    void clear_line(int y);
    
    // Render to terminal - returns escape sequence string
    std::string flush();
    
    // Draw primitives
    void draw_box(int x, int y, int w, int h, const Style& style);
    void draw_hline(int x, int y, int len, char32_t ch = U'─');
    void draw_vline(int x, int y, int len, char32_t ch = U'│');

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<Cell> front_buffer_;
    std::vector<Cell> back_buffer_;
    Style current_style_;
    
    size_t index(int x, int y) const { return y * width_ + x; }
    bool in_bounds(int x, int y) const {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }
    
    std::string style_to_escape(const Style& style);
    std::string char32_to_utf8(char32_t ch);
};

}  // namespace catvim
