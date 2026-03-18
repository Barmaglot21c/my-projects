#include "svg.h"

#include <memory>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace svg {

using namespace std;

std::ostream& operator<<(std::ostream& os, const Color& color) {
  return std::visit(ColorToStream(os), color);
}

std::ostream& operator<<(std::ostream& os, StrokeLineCap cap) {
  switch (cap) {
    case StrokeLineCap::BUTT:
      os << "butt";
      break;
    case StrokeLineCap::ROUND:
      os << "round";
      break;
    case StrokeLineCap::SQUARE:
      os << "square";
      break;
    default:
      return os;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, StrokeLineJoin join) {
  switch (join) {
    case StrokeLineJoin::ARCS:
      os << "arcs";
      break;
    case StrokeLineJoin::BEVEL:
      os << "bevel";
      break;
    case StrokeLineJoin::MITER:
      os << "miter";
      break;
    case StrokeLineJoin::MITER_CLIP:
      os << "miter-clip";
      break;
    case StrokeLineJoin::ROUND:
      os << "round";
      break;
    default:
      return os;
  }
  return os;
}

// ---------- Document -----------------

void Document::Render(std::ostream& out) const {
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

  RenderContext ctx = {out, indent_step_, indent_step_};
  for (const auto& obj_ptr : objects_) {
    obj_ptr->Render(ctx);
  }

  out << "</svg>"sv;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
  if (obj)
    objects_.push_back(move(obj));
}

void Document::SetIndentStep(int indent) {
  indent_step_ = indent >= 0 ? indent : throw invalid_argument("Табуляция не может быть отрицательной");
}

int Document::GetIndentStep() const {
  return indent_step_;
}

// ---------- Object ------------------

void Object::Render(const RenderContext& context) const {
  context.RenderIndent();

  // Делегируем вывод тега своим подклассам
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
  out << "<circle"sv;
  RenderAttr(out, "cx", center_.x);
  RenderAttr(out, "cy", center_.y);
  RenderAttr(out, "r", radius_);
  RenderPathProps(out);
  out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
  points_.push_back(point);
  return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
  auto& out = context.out;
  out << "<polyline"sv;
  out << " points=\""sv;
  for (bool is_first = true; Point point : points_) {
    if (!is_first) {
      out << " "sv;
    } else
      is_first = false;
    out << point.x << ","sv << point.y;
  }
  out << "\""sv;
  RenderPathProps(out);
  out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
  position_ = pos;
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
  font_family_ = font_family;
  return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
  font_weight_ = font_weight;
  return *this;
}

Text& Text::SetData(std::string data) {
  data_ = data;
  return *this;
}

string EncodeToXml(const string& data) {
  static unordered_map<char, string> key_words = {
      {'\"', "&quot;"},
      {'\'', "&apos;"},
      {'<',  "&lt;"  },
      {'>',  "&gt;"  },
      {'&',  "&amp;" },
  };
  string result;
  for (char c : data) {
    if (auto it = key_words.find(c); it != key_words.end()) {
      result.append(it->second);
    } else {
      result.push_back(c);
    }
  }
  return result;
}

void Text::RenderObject(const RenderContext& context) const {
  auto& out = context.out;

  out << "<text";

  RenderPathProps(out);
  
  RenderAttr(out, "x", position_.x);
  RenderAttr(out, "y", position_.y);
  RenderAttr(out, "dx", offset_.x);
  RenderAttr(out, "dy", offset_.y);
  RenderAttr(out, "font-size", font_size_);
  if (!font_family_.empty())
    RenderAttr(out, "font-family", font_family_);
  if (!font_weight_.empty())
    RenderAttr(out, "font-weight", font_weight_);

  out << ">";

  out << move(EncodeToXml(data_));

  out << "</text>";
}

}  // namespace svg