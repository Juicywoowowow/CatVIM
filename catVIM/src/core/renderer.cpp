#include "renderer.hpp"
#include <sstream>
#include <cstring>

namespace catvim {

bool Style::operator==(const Style& other) const {
    if (fg.is_default != other.fg.is_default) return false;
    if (!fg.is_default && (fg.r != other.fg.r || fg.g != other.fg.g || fg.b != other.fg.b)) return false;
    if (bg.is_default != other.bg.is_default) return false;
    if (!bg.is_default && (bg.r != other.bg.r || bg.g != other.bg.g || bg.b != other.bg.b)) return false;
    return attrs == other.attrs;
}

Renderer::Renderer() {}

void Renderer::resize(int width, int height) {
    width_ = width;
    height_ = height;
    front_buffer_.resize(width * height);
    back_buffer_.resize(width * height);
    clear();
}

void Renderer::set_cell(int x, int y, char32_t ch, const Style& style) {
    if (!in_bounds(x, y)) return;
    back_buffer_[index(x, y)] = {ch, style};
}

void Renderer::set_cell(int x, int y, char32_t ch) {
    set_cell(x, y, ch, current_style_);
}

void Renderer::set_string(int x, int y, const std::string& str, const Style& style) {
    for (size_t i = 0; i < str.size() && x + static_cast<int>(i) < width_; i++) {
        set_cell(x + i, y, static_cast<char32_t>(str[i]), style);
    }
}

void Renderer::clear() {
    Cell empty = {' ', Style{}};
    std::fill(back_buffer_.begin(), back_buffer_.end(), empty);
}

void Renderer::clear_line(int y) {
    if (y < 0 || y >= height_) return;
    Cell empty = {' ', Style{}};
    for (int x = 0; x < width_; x++) {
        back_buffer_[index(x, y)] = empty;
    }
}

std::string Renderer::char32_to_utf8(char32_t cp) {
    std::string result;
    if (cp < 0x80) {
        result += static_cast<char>(cp);
    } else if (cp < 0x800) {
        result += static_cast<char>(0xC0 | (cp >> 6));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        result += static_cast<char>(0xE0 | (cp >> 12));
        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        result += static_cast<char>(0xF0 | (cp >> 18));
        result += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    }
    return result;
}

std::string Renderer::style_to_escape(const Style& style) {
    std::ostringstream oss;
    oss << "\x1b[0";  // Reset
    
    if (style.attrs & Attr::BOLD) oss << ";1";
    if (style.attrs & Attr::DIM) oss << ";2";
    if (style.attrs & Attr::ITALIC) oss << ";3";
    if (style.attrs & Attr::UNDERLINE) oss << ";4";
    if (style.attrs & Attr::BLINK) oss << ";5";
    if (style.attrs & Attr::REVERSE) oss << ";7";
    if (style.attrs & Attr::STRIKETHROUGH) oss << ";9";
    
    if (!style.fg.is_default) {
        oss << ";38;2;" << (int)style.fg.r << ";" << (int)style.fg.g << ";" << (int)style.fg.b;
    }
    if (!style.bg.is_default) {
        oss << ";48;2;" << (int)style.bg.r << ";" << (int)style.bg.g << ";" << (int)style.bg.b;
    }
    
    oss << "m";
    return oss.str();
}

std::string Renderer::flush() {
    std::ostringstream out;
    
    Style last_style;
    bool first = true;
    
    for (int y = 0; y < height_; y++) {
        bool line_changed = false;
        for (int x = 0; x < width_; x++) {
            if (back_buffer_[index(x, y)] != front_buffer_[index(x, y)]) {
                line_changed = true;
                break;
            }
        }
        
        if (!line_changed) continue;
        
        // Move to start of line
        out << "\x1b[" << (y + 1) << ";1H";
        
        for (int x = 0; x < width_; x++) {
            const Cell& cell = back_buffer_[index(x, y)];
            
            if (first || cell.style != last_style) {
                out << style_to_escape(cell.style);
                last_style = cell.style;
                first = false;
            }
            
            out << char32_to_utf8(cell.ch);
        }
    }
    
    // Reset style at end
    out << "\x1b[0m";
    
    // Swap buffers
    front_buffer_ = back_buffer_;
    
    return out.str();
}

void Renderer::draw_box(int x, int y, int w, int h, const Style& style) {
    if (w < 2 || h < 2) return;
    
    // Corners
    set_cell(x, y, U'┌', style);
    set_cell(x + w - 1, y, U'┐', style);
    set_cell(x, y + h - 1, U'└', style);
    set_cell(x + w - 1, y + h - 1, U'┘', style);
    
    // Horizontal lines
    for (int i = 1; i < w - 1; i++) {
        set_cell(x + i, y, U'─', style);
        set_cell(x + i, y + h - 1, U'─', style);
    }
    
    // Vertical lines
    for (int i = 1; i < h - 1; i++) {
        set_cell(x, y + i, U'│', style);
        set_cell(x + w - 1, y + i, U'│', style);
    }
}

void Renderer::draw_hline(int x, int y, int len, char32_t ch) {
    for (int i = 0; i < len; i++) {
        set_cell(x + i, y, ch);
    }
}

void Renderer::draw_vline(int x, int y, int len, char32_t ch) {
    for (int i = 0; i < len; i++) {
        set_cell(x, y + i, ch);
    }
}

}  // namespace catvim
