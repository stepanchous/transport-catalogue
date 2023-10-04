#include "svg.h"

#include <string>

namespace svg {

using namespace std::literals;

Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}

Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double op)
    : red(r), green(g), blue(b), opacity(op) {}

void ColorPrinter::operator()(std::monostate) { out << "none"sv; }
void ColorPrinter::operator()(const std::string& color) { out << color; }
void ColorPrinter::operator()(Rgb rgb) {
    out << "rgb("sv << static_cast<uint32_t>(rgb.red) << ","sv
        << static_cast<uint32_t>(rgb.green) << ","sv
        << static_cast<uint32_t>(rgb.blue) << ")"sv;
}
void ColorPrinter::operator()(Rgba rgba) {
    out << "rgba("sv << static_cast<uint32_t>(rgba.red) << ","sv
        << static_cast<uint32_t>(rgba.green) << ","sv
        << static_cast<uint32_t>(rgba.blue) << ","sv << rgba.opacity << ")"sv;
}

std::ostream& operator<<(std::ostream& out, Color color) {
    std::visit(ColorPrinter{out}, color);
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap) {
    switch (stroke_line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join) {
    switch (stroke_line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
    }
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    RenderObject(context);
    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle";
    out << " cx=\""sv << center_.x << "\""sv;
    out << " cy=\""sv << center_.y << "\""sv;
    out << " r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points="sv;
    FormatPoints(out);
    RenderAttrs(out);
    out << "/>"sv;
}

void Polyline::FormatPoints(std::ostream& out) const {
    bool first_point = true;
    out << "\"";
    for (const Point& point : points_) {
        if (first_point) {
            out << point.x << ","sv << point.y;
            first_point = false;
            continue;
        }
        out << " "sv << point.x << ","sv << point.y;
    }
    out << "\"";
}

// ---------- Text ------------------

const std::unordered_map<char, std::string> Text::special_char_to_escaping_seq =
    {{'\"', "&quot;"},
     {'\'', "&apos;"},
     {'<', "&lt;"},
     {'>', "&gt;"},
     {'&', "&amp;"}};

Text& Text::SetPosition(Point position) {
    position_ = position;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv << position_.x << "\""sv;
    out << " y=\""sv << position_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\""sv;
    out << " dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << font_size_ << "\""sv;
    if (font_family_) {
        out << " font-family=\""sv;
        FormatStringData(*font_family_, out);
        out << "\""sv;
    }
    if (font_weight_) {
        out << " font-weight=\""sv;
        FormatStringData(*font_weight_, out);
        out << "\""sv;
    }
    out << ">"sv;
    FormatStringData(data_, out);
    out << "</text>"sv;
}

void Text::FormatStringData(const std::string& data, std::ostream& out) {
    for (char c : data) {
        if (special_char_to_escaping_seq.count(c)) {
            out << special_char_to_escaping_seq.at(c);
            continue;
        }
        out << c;
    }
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    RenderContext context = RenderContext(out, 2, 0);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
    for (const auto& object : objects_) {
        object->Render(context.Indented());
    }
    out << "</svg>";
}

}  // namespace svg
