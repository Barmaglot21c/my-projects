#include "sheet.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>

#include "cell.h"
#include "common.h"

using namespace std;

std::ostream& operator<<(std::ostream& os, const CellInterface::Value& value) {
  std::visit([&os](const auto& val) { os << val; }, value);
  return os;
}

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text) {
  if (!pos.IsValid()) {
    throw InvalidPositionException("Invalid pos during set cell");
  }

  unique_ptr<CellInterface>& cell = cells_[pos];
  if (!cell) {
    cell = make_unique<Cell>(*this);
  }

  static_cast<Cell*>(cell.get())->Set(text);

  // printable area
  if (!text.empty()) {
    printable_size_.rows = max(printable_size_.rows, pos.row + 1);
    printable_size_.cols = max(printable_size_.cols, pos.col + 1);
  }
}

const CellInterface* Sheet::GetCell(Position pos) const {
  if (!pos.IsValid()) {
    throw InvalidPositionException("Invalid pos during get");
  }
  return cells_.count(pos) ? cells_.at(pos).get() : nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
  if (!pos.IsValid()) {
    throw InvalidPositionException("Invalid pos during get");
  }
  return cells_.count(pos) ? cells_.at(pos).get() : nullptr;
}

void Sheet::ClearCell(Position pos) {
  if (!pos.IsValid()) {
    throw InvalidPositionException("Invalid pos during clear cell");
  }
  cells_[pos] = nullptr;

  // printable area
  if (pos.row + 1 == printable_size_.rows) {
    auto new_max_row = max_element(cells_.begin(), cells_.end(), [](const auto& lhs, const auto& rhs) {
      return lhs.first.row < rhs.first.row || !lhs.second;
    });
    if (new_max_row->second) {
      printable_size_.rows = new_max_row->first.row + 1;
    } else {
      printable_size_ = {0, 0};
    }
  }
  if (pos.col + 1 == printable_size_.cols) {
    auto new_max_col = max_element(cells_.begin(), cells_.end(), [](const auto& lhs, const auto& rhs) {
      return lhs.first.col < rhs.first.col || !lhs.second;
    });
    printable_size_.cols = new_max_col->first.col + 1;
  }
}

Size Sheet::GetPrintableSize() const {
  return printable_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
  for (int y = 0; y < printable_size_.rows; ++y) {
    for (int x = 0; x < printable_size_.cols; ++x) {
      if (x != 0) {
        output << '\t';
      }
      if (cells_.count({y, x}) && cells_.at({y, x})) {
        output << cells_.at({y, x})->GetValue();
      }
    }
    output << '\n';
  }
}
void Sheet::PrintTexts(std::ostream& out) const {
  stringstream output;
  for (int y = 0; y < printable_size_.rows; ++y) {
    for (int x = 0; x < printable_size_.cols; ++x) {
      if (x != 0) {
        output << '\t';
      }
      if (cells_.count({y, x}) && cells_.at({y, x})) {
        output << cells_.at({y, x})->GetText();
      }
    }
    output << '\n';
  }
  string i = output.str();
  out << i;
}

std::unique_ptr<SheetInterface> CreateSheet() {
  return std::make_unique<Sheet>();
}