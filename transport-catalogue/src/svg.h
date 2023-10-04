#pragma once

#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace svg {

struct Rgb {
    Rgb(uint8_t r, uint8_t g, uint8_t b);
    Rgb() = default;

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba {
    Rgba(uint8_t r, uint8_t g, uint8_t b, double op);
    Rgba() = default;

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const std::string NoneColor = "none";

struct ColorPrinter {
    void operator()(std::monostate);
    void operator()(const std::string& color);
    void operator()(Rgb rgb);
    void operator()(Rgba rgba);

    std::ostream& out;
};

std::ostream& operator<<(std::ostream& out, Color color);

struct Point {
    Point() = default;
    Point(double x, double y) : x(x), y(y) {}
    double x = 0;
    double y = 0;
};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap);

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join);

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с
 * отступами. Хранит ссылку на поток вывода, текущее значение и шаг отступа при
 * выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out) : out(out) {}
    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out), indent_step(indent_step), indent(indent) {}
    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
   public:
    void Render(const RenderContext& context) const;
    virtual ~Object() = default;

   private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

class ObjectContainer {
   public:
    virtual ~ObjectContainer() = default;
    virtual void AddPtr(std::unique_ptr<Object>&& object) = 0;

    template <typename Obj>
    void Add(Obj object);
};

class Drawable {
   public:
    virtual ~Drawable() = default;
    virtual void Draw(ObjectContainer& object_container) const = 0;
};

template <typename Owner>
class PathProps {
   public:
    Owner& SetFillColor(Color fill_color);
    Owner& SetStrokeColor(Color stroke_color);
    Owner& SetStrokeWidth(double width);
    Owner& SetStrokeLineCap(StrokeLineCap line_cap);
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join);

   protected:
    ~PathProps() = default;
    void RenderAttrs(std::ostream& out) const;

   private:
    std::optional<Color> fill_color_ = std::nullopt;
    std::optional<Color> stroke_color_ = std::nullopt;
    std::optional<double> stroke_width_ = std::nullopt;
    std::optional<StrokeLineCap> stroke_line_cap_ = std::nullopt;
    std::optional<StrokeLineJoin> stroke_line_join_ = std::nullopt;

    Owner& AsOwner();
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
   public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

   private:
    Point center_ = {0.0, 0.0};
    double radius_ = 1.0;

    void RenderObject(const RenderContext& context) const override;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
   public:
    Polyline& AddPoint(Point point);

   private:
    std::vector<Point> points_ = {};

    void RenderObject(const RenderContext& context) const override;
    void FormatPoints(std::ostream& context) const;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text : public Object, public PathProps<Text> {
   public:
    Text& SetPosition(Point position);
    Text& SetOffset(Point offset);
    Text& SetFontSize(uint32_t size);
    Text& SetFontFamily(std::string font_family);
    Text& SetFontWeight(std::string font_weight);
    Text& SetData(std::string data);

   private:
    Point position_ = {0.0, 0.0};
    Point offset_ = {0.0, 0.0};
    uint32_t font_size_ = 1;
    std::optional<std::string> font_family_ = std::nullopt;
    std::optional<std::string> font_weight_ = std::nullopt;
    std::string data_ = "";
    const static std::unordered_map<char, std::string>
        special_char_to_escaping_seq;

    void RenderObject(const RenderContext& context) const override;
    static void FormatStringData(const std::string& data, std::ostream& out);
};

class Document final : public ObjectContainer {
   public:
    void AddPtr(std::unique_ptr<Object>&& obj) override;
    void Render(std::ostream& out) const;

   private:
    std::deque<std::unique_ptr<Object>> objects_;
};

template <typename Obj>
void ObjectContainer::Add(Obj object) {
    AddPtr(std::make_unique<Obj>(std::move(object)));
}

template <typename Owner>
Owner& PathProps<Owner>::SetFillColor(Color fill_color) {
    fill_color_ = std::move(fill_color);
    return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeColor(Color stroke_color) {
    stroke_color_ = std::move(stroke_color);
    return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeWidth(double width) {
    stroke_width_ = width;
    return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap stroke_line_cap) {
    stroke_line_cap_ = stroke_line_cap;
    return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin stroke_line_join) {
    stroke_line_join_ = stroke_line_join;
    return AsOwner();
}

template <typename Owner>
void PathProps<Owner>::RenderAttrs(std::ostream& out) const {
    using namespace std::literals;

    if (fill_color_) {
        out << " fill=\"" << *fill_color_ << "\""sv;
    }
    if (stroke_color_) {
        out << " stroke=\"" << *stroke_color_ << "\""sv;
    }
    if (stroke_width_) {
        out << " stroke-width=\"" << *stroke_width_ << "\""sv;
    }
    if (stroke_line_cap_) {
        out << " stroke-linecap=\"" << *stroke_line_cap_ << "\""sv;
    }
    if (stroke_line_join_) {
        out << " stroke-linejoin=\"" << *stroke_line_join_ << "\""sv;
    }
}

template <typename Owner>
Owner& PathProps<Owner>::AsOwner() {
    return static_cast<Owner&>(*this);
}

}  // namespace svg
