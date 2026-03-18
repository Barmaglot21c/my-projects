#pragma once

#include <functional>
#include <iostream>
#include <unordered_map>

#include "cell.h"
#include "common.h"

class Sheet : public SheetInterface {
 public:
  ~Sheet();

  void SetCell(Position pos, std::string text) override;

  const CellInterface* GetCell(Position pos) const override;
  CellInterface* GetCell(Position pos) override;

  void ClearCell(Position pos) override;

  Size GetPrintableSize() const override;

  void PrintValues(std::ostream& output) const override;
  void PrintTexts(std::ostream& output) const override;

 private:
  std::unordered_map<Position, std::unique_ptr<CellInterface>> cells_;
  Size printable_size_;
};