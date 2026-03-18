#pragma once

#include <unordered_set>
#include <memory>

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
 public:
  Cell(SheetInterface& sheet);
  ~Cell();

  void Set(std::string text);
  void Clear();

  Value GetValue() const override;
  std::string GetText() const override;
  inline std::vector<Position> GetReferencedCells() const override {
    return referenced_cells_;
  }
  inline bool IsReferenced() const {
    return !referenced_cells_.empty();
  }

 private:
  class Impl;
  class EmptyImpl;
  class TextImpl;
  class FormulaImpl;
  // Рекурентно сбрасываем кэш
  void DropDependentCache();                                                      // Инициирующий
  void DropDependentCache(std::unordered_set<const Cell*>& dropped_cache_cells);  // Рекурсивный
  // Проверка зацикливания
  void LoopProof(const std::unique_ptr<FormulaImpl>& new_impl) const;  // Инициирующий
  void LoopProof(const std::vector<Position>& referenced_cells,
                 std::unordered_set<const Cell*>& filled_cells) const;

  void UpdateCellsDependencies(const std::unique_ptr<FormulaImpl>& new_impl = nullptr);

 private:
  std::unique_ptr<Impl> impl_;
  SheetInterface& sheet_;
  std::unordered_set<Cell*> dependent_cells_;  // for cache
  std::vector<Position> referenced_cells_;     // for evaluate
};