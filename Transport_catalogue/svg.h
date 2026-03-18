#pragma once

#include <cmath>
#include <cstdint>
#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

struct Rgb {
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
};
struct Rgba {
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  double opacity = 1;
};

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
inline const Color NoneColor;

struct ColorToStream {
  std::ostream& os;
  ColorToStream(std::ostream& _os) : os(_os) {}

  std::ostream& operator()(std::monostate) const {
    using namespace std::literals;
    os << "none"s;
    return os;
  }
  std::ostream& operator()(const std::string& color_name) const {
    os << color_name;
    return os;
  }
  std::ostream& operator()(const Rgb& value) const {
    using namespace std;
    os << "rgb("s << int(value.red) << ","s << int(value.green) << ","s << int(value.blue) << ")"s;
    return os;
  }
  std::ostream& operator()(const Rgba& value) const {
    using namespace std;

    os << "rgba("s << int(value.red) << ","s << int(value.green) << ","s << int(value.blue) << ","s
       << value.opacity << ")"s;
    return os;
  }
};

std::ostream& operator<<(std::ostream& os, const Color& color);

enum class StrokeLineCap {
  BUTT,
  ROUND,
  SQUARE,
};

enum class StrokeLineJoin {
  ARCS,
  BEVEL,
  MITER,
  MITER_CLIP,
  ROUND,
};

std::ostream& operator<<(std::ostream& os, StrokeLineCap cap);
std::ostream& operator<<(std::ostream& os, StrokeLineJoin join);

struct Point {
  Point() = default;
  Point(double x, double y) : x(x), y(y) {}

  double x = 0;
  double y = 0;
};

template <typename AttrType>
void RenderAttr(std::ostream& out, std::string_view name, const AttrType& value) {
  using namespace std::literals;
  out << " "sv << name << "=\""sv;
  out << value;
  out.put('"');
}

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
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

template <typename Owner>
class PathProps {
 public:
  Owner& SetFillColor(Color fill_color) {
    fill_color_ = std::move(fill_color);
    return AsOwner();
  }
  Owner& SetStrokeColor(Color stroke_color) {
    stroke_color_ = std::move(stroke_color);
    return AsOwner();
  }
  Owner& SetStrokeWidth(double width) {
    stroke_width_ = width;
    return AsOwner();
  }
  Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
    line_cap_ = line_cap;
    return AsOwner();
  }
  Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
    line_join_ = line_join;
    return AsOwner();
  }

  virtual void RenderPathProps(std::ostream& out) const {
    if (fill_color_) {
      RenderAttr(out, "fill", fill_color_.value());
    }
    if (stroke_color_) {
      RenderAttr(out, "stroke", stroke_color_.value());
    }
    if (stroke_width_) {
      RenderAttr(out, "stroke-width", stroke_width_.value());
    }
    if (line_cap_) {
      RenderAttr(out, "stroke-linecap", line_cap_.value());
    }
    if (line_join_) {
      RenderAttr(out, "stroke-linejoin", line_join_.value());
    }
  }

 protected:
  ~PathProps() = default;

 private:
  Owner& AsOwner() {
    return static_cast<Owner&>(*this);
  }

  std::optional<Color> fill_color_;
  std::optional<Color> stroke_color_;
  std::optional<double> stroke_width_;
  std::optional<StrokeLineCap> line_cap_;
  std::optional<StrokeLineJoin> line_join_;
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
  void RenderObject(const RenderContext& context) const override;

  Point center_;
  double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline : public Object, public PathProps<Polyline> {
 public:
  // Добавляет очередную вершину к ломаной линии
  Polyline& AddPoint(Point point);

  /*
   * Прочие методы и данные, необходимые для реализации элемента <polyline>
   */

 private:
  void RenderObject(const RenderContext& context) const override;

  std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text : public Object, public PathProps<Text> {
 public:
  // Задаёт координаты опорной точки (атрибуты x и y)
  Text& SetPosition(Point pos);

  // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
  Text& SetOffset(Point offset);

  // Задаёт размеры шрифта (атрибут font-size)
  Text& SetFontSize(uint32_t size);

  // Задаёт название шрифта (атрибут font-family)
  Text& SetFontFamily(std::string font_family);

  // Задаёт толщину шрифта (атрибут font-weight)
  Text& SetFontWeight(std::string font_weight);

  // Задаёт текстовое содержимое объекта (отображается внутри тега text)
  Text& SetData(std::string data);

  // Прочие данные и методы, необходимые для реализации элемента <text>

 private:
  void RenderObject(const RenderContext& context) const override;

  Point position_;
  Point offset_;
  uint32_t font_size_ = 1;
  std::string font_family_;
  std::string font_weight_;
  std::string data_;
};

class ObjectContainer {
 public:
  /*
   Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
   Пример использования:
   Document doc;
   doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
  */
  template <typename SpecificObject>
  void Add(const SpecificObject& obj);

  // Добавляет в svg-документ объект-наследник svg::Object
  virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
  virtual ~ObjectContainer() = default;
};

class Document : public ObjectContainer {
 public:
  // Добавляет в svg-документ объект-наследник svg::Object
  void AddPtr(std::unique_ptr<Object>&& obj);

  // Выводит в ostream svg-представление документа
  void Render(std::ostream& out) const;

  // Прочие методы и данные, необходимые для реализации класса Document
  void SetIndentStep(int indent);
  int GetIndentStep() const;

 private:
  std::vector<std::unique_ptr<Object>> objects_;
  int indent_step_ = 2;
};

class Drawable {
 public:
  virtual void Draw(ObjectContainer& container) const = 0;
  virtual ~Drawable() = default;
};

template <typename SpecificObject>
void ObjectContainer::Add(const SpecificObject& obj) {
  AddPtr(std::make_unique<SpecificObject>(obj));
}

}  // namespace svg