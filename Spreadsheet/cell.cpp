#include "cell.h"

#include <cassert>
#include <iostream>
#include <optional>
#include <string>

using namespace std;

class Cell::Impl {
 public:
  virtual ~Impl() = default;

  virtual Value GetValue() const = 0;
  virtual string GetText() const = 0;
  virtual void DropCache() = 0;
};

class Cell::EmptyImpl : public Impl {
 public:
  EmptyImpl() = default;

  string GetText() const override {
    return {};
  }

  Value GetValue() const override {
    return GetText();
  }

  void DropCache() override {}
};

class Cell::TextImpl : public Impl {
 public:
  TextImpl(string str) : text_(move(str)) {}

  string GetText() const override {
    return text_;
  }

  Value GetValue() const override {
    return text_.at(0) == ESCAPE_SIGN ? text_.substr(1) : text_;
  }

  void DropCache() override {}

 private:
  string text_;
};

class Cell::FormulaImpl : public Impl {
 public:
  FormulaImpl(SheetInterface& sheet, unique_ptr<FormulaInterface> fm)
      : sheet_(sheet), formula_(move(fm)) {}

  string GetText() const override {
    return FORMULA_SIGN + formula_->GetExpression();
  }

  Value GetValue() const override {
    if (!cache_) {
      cache_ = formula_->Evaluate(sheet_);
    }
    if (holds_alternative<double>(*cache_)) {
      return get<double>(*cache_);
    }
    return get<FormulaError>(*cache_);
  }

  vector<Position> GetReferencedCells() const {
    return formula_->GetReferencedCells();
  }

  void DropCache() override {
    cache_ = nullopt;
  }

 private:
  SheetInterface& sheet_;
  unique_ptr<FormulaInterface> formula_;
  mutable std::optional<FormulaInterface::Value> cache_;
};

// Реализуйте следующие методы
Cell::Cell(SheetInterface& sheet) : sheet_(sheet) {}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
  if (text.empty()) {
    UpdateCellsDependencies();
    impl_ = make_unique<EmptyImpl>();
  } else if (text.at(0) == FORMULA_SIGN && text.size() != 1) {
    unique_ptr<FormulaImpl> new_formula =
        make_unique<FormulaImpl>(sheet_, ParseFormula(text.erase(0, 1)));

    LoopProof(new_formula);
    DropDependentCache();
    UpdateCellsDependencies(new_formula);

    impl_ = move(new_formula);
  } else {
    UpdateCellsDependencies();
    impl_ = make_unique<TextImpl>(text);
  }
}

void Cell::Clear() {
  impl_ = make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
  return impl_->GetValue();
}
std::string Cell::GetText() const {
  return impl_->GetText();
}

void Cell::DropDependentCache() {
  std::unordered_set<const Cell*> dropped_cells = {};
  DropDependentCache(dropped_cells);
}
void Cell::DropDependentCache(std::unordered_set<const Cell*>& dropped_cache_cells) {
  for (Cell* dependent_cell : dependent_cells_) {
    if (dropped_cache_cells.find(dependent_cell) == dropped_cache_cells.end()) {
      dependent_cell->impl_->DropCache();
      dropped_cache_cells.insert(dependent_cell);
      dependent_cell->DropDependentCache(dropped_cache_cells);
    }
  }
}

void Cell::LoopProof(const unique_ptr<FormulaImpl>& new_impl) const {
  std::unordered_set<const Cell*> filled_cells = {};
  LoopProof(new_impl->GetReferencedCells(), filled_cells);
}
void Cell::LoopProof(const std::vector<Position>& referenced_cells,
                     std::unordered_set<const Cell*>& filled_cells) const {
  for (const Position pos : referenced_cells) {
    const Cell* ref_cell = static_cast<Cell*>(sheet_.GetCell(pos));
    if (this == ref_cell) {
      throw CircularDependencyException("Get looped");
    } else if (ref_cell && filled_cells.find(ref_cell) == filled_cells.end()) {
      filled_cells.insert(ref_cell);
      LoopProof(ref_cell->GetReferencedCells(), filled_cells);
    }
  }
}

void Cell::UpdateCellsDependencies(const unique_ptr<FormulaImpl>& new_impl) {
  // Удаляем старые зависимости
  for (const Position& pos : referenced_cells_) {
    Cell* cell = static_cast<Cell*>(sheet_.GetCell(pos));
    cell->dependent_cells_.erase(this);
  }

  if (!new_impl){
    referenced_cells_.clear();
    return;
  }

  // Прокидываем новые
  referenced_cells_ = new_impl->GetReferencedCells();
  for (const Position& pos : referenced_cells_) {
    Cell* cell = static_cast<Cell*>(sheet_.GetCell(pos));
    if (!cell) {
      sheet_.SetCell(pos, ""s);
      cell = static_cast<Cell*>(sheet_.GetCell(pos));
    }
    cell->dependent_cells_.insert(this);
  }
}