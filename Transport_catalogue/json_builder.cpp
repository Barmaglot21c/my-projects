#include "json_builder.h"

#include <utility>

using namespace std;

namespace json {

Builder::KeyItemContext Builder::Key(std::string key) {
  if (nodes_stack_.empty()) {
    throw logic_error("Документ уже закончен");
  }
  auto& last_node = nodes_stack_.back();
  if (!last_node->IsDict() || cur_key_) {
    throw logic_error("Неверно задан ключ");
  }
  cur_key_ = move(key);
  return *this;
}

Builder& Builder::Value(Node node) {
  if (nodes_stack_.empty()) {
    throw logic_error("Документ уже закончен");
  }
  auto& last_node = nodes_stack_.back();
  if (last_node->IsNull()) {
    *nodes_stack_.back() = move(node);
    nodes_stack_.clear();
  } else if (last_node->IsArray()) {
    auto& array = get<Array>(nodes_stack_.back()->GetValue());
    array.emplace_back(move(node));
  } else if (last_node->IsDict()) {
    auto& dict = get<Dict>(nodes_stack_.back()->GetValue());
    if (!cur_key_) {
      throw logic_error("Не задан ключ");
    }
    dict.emplace(move(*cur_key_), move(node));
    cur_key_.reset();
  }
  return *this;
}

Builder::DictItemContext Builder::StartDict() {
  if (nodes_stack_.empty()) {
    throw logic_error("Документ уже закончен");
  }
  auto& last_node = nodes_stack_.back();
  if (last_node->IsNull()) {
    root_ = Dict{};
  } else if (last_node->IsArray()) {
    auto& array = get<Array>(last_node->GetValue());
    array.emplace_back(Dict{});
    nodes_stack_.push_back(&array.back());
  } else if (last_node->IsDict()) {
    if (!cur_key_) {
      throw logic_error("Не задан ключ");
    }
    auto& dict = get<Dict>(last_node->GetValue());
    dict.emplace(*cur_key_, Dict{});
    nodes_stack_.push_back(&dict[*cur_key_]);
    cur_key_.reset();
  }

  return *this;
}

Builder::ArrayItemContext Builder::StartArray() {
  if (nodes_stack_.empty()) {
    throw logic_error("Документ уже закончен");
  }
  auto& last_node = nodes_stack_.back();
  if (last_node->IsNull()) {
    root_ = Array{};
  } else if (last_node->IsArray()) {
    auto& array = get<Array>(last_node->GetValue());
    array.emplace_back(Array{});
    nodes_stack_.push_back(&array.back());
  } else if (last_node->IsDict()) {
    if (!cur_key_) {
      throw logic_error("Не задан ключ");
    }
    auto& dict = get<Dict>(last_node->GetValue());
    dict.emplace(*cur_key_, Array{});
    nodes_stack_.push_back(&dict[*cur_key_]);
    cur_key_.reset();
  }

  return *this;
}

Builder& Builder::EndDict() {
  if (nodes_stack_.empty()) {
    throw logic_error("Документ уже закончен");
  }
  auto& last_node = nodes_stack_.back();
  if (!last_node->IsDict() || cur_key_) {
    throw logic_error("Неверный конец словаря");
  }
  nodes_stack_.pop_back();
  return *this;
}

Builder& Builder::EndArray() {
  if (nodes_stack_.empty()) {
    throw logic_error("Документ уже закончен");
  }
  auto& last_node = nodes_stack_.back();
  if (!last_node->IsArray()) {
    throw logic_error("Неверный конец массива");
  }
  nodes_stack_.pop_back();
  return *this;
}

Node Builder::Build() {
  if (!nodes_stack_.empty()) {
    throw logic_error("Документ еще не закончен");
  }
  return move(root_);
}

}  // namespace json