#include "formula.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <sstream>

#include "FormulaAST.h"

using namespace std;

std::ostream& operator<<(std::ostream& output, const FormulaError& fe) {
  return output << fe.ToString();
}

template <typename T>
vector<T> MakeUniqueVector(const forward_list<T>& flist) {
  if (flist.empty())
    return {};

  size_t count = 0;
  for (auto it = flist.begin(); it != flist.end(); ++it) {
    ++count;
  }

  std::unordered_set<T> unique_set;
  unique_set.reserve(count);

  std::vector<T> result;
  result.reserve(count);

  for (const auto& item : flist) {
    if (unique_set.insert(item).second) {
      result.push_back(item);
    }
  }

  result.shrink_to_fit();
  return result;
}

namespace {
class Formula : public FormulaInterface {
 private:
  struct UnpackValue {
    double operator()(double value) {
      return value;
    }

    double operator()(const std::string& text) {
      double value;
      try {
        value = std::stod(text);
      } catch (const std::invalid_argument&) {
        throw FormulaError(FormulaError::Category::Value);
      }
      return value;
    }

    double operator()(const FormulaError& error) {
      throw error;
    }
  };

 public:
  explicit Formula(std::string expression) try : ast_(ParseFormulaAST(std::move(expression))) {
  } catch (const FormulaException& fx) {
    throw;
  }

  Value Evaluate(const SheetInterface& sheet) const override {
    // Функтор-геетер значения ячеки по адресу
    auto value_getter = [&sheet](Position pos) {
      if (!pos.IsValid())
        throw FormulaError(FormulaError::Category::Ref);

      const CellInterface* cell = sheet.GetCell(pos);
      if (!cell)
        return .0;
      const CellInterface::Value value = cell->GetValue();
      if (const string* str = get_if<string>(&value); str && str->empty())
        return .0;

      // Value to double
      return visit(UnpackValue(), value);
    };

    try {
      return ast_.Execute(value_getter);
    } catch (const FormulaError& fe) {
      return fe;
    }
  }
  std::string GetExpression() const override {
    std::ostringstream oss;
    ast_.PrintFormula(oss);
    return oss.str();
  }

  std::vector<Position> GetReferencedCells() const override {
    const auto& referenced_cells = ast_.GetCells();
    return MakeUniqueVector<Position>(referenced_cells);
  }

 private:
  FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
  return std::make_unique<Formula>(std::move(expression));
}
