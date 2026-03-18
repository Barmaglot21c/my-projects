#pragma once

#include <optional>
#include <string>
#include <vector>

#include "json.h"

namespace json {

class Builder {
  class DictItemContext;
  class ArrayItemContext;
  class KeyItemContext;

 private:
  class BaseContext {
   public:
    BaseContext(Builder& builder) : builder_(builder) {}
    KeyItemContext Key(std::string key) {
      return builder_.Key(key);
    }
    Builder& Value(Node node) {
      return builder_.Value(node);
    }
    DictItemContext StartDict() {
      return builder_.StartDict();
    }
    ArrayItemContext StartArray() {
      return builder_.StartArray();
    }
    Builder& EndDict() {
      return builder_.EndDict();
    }
    Builder& EndArray() {
      return builder_.EndArray();
    }
    Node Build() {
      return builder_.Build();
    }
   private:
    Builder& builder_;
  };

  class KeyItemContext : BaseContext {
   public:
    KeyItemContext(Builder& builder) : BaseContext(builder) {}

    using BaseContext::StartArray;
    using BaseContext::StartDict;

    DictItemContext Value(Node node) {
      return BaseContext::Value(node);
    }
  };

  class DictItemContext : BaseContext {
   public:
    DictItemContext(Builder& builder) : BaseContext(builder) {}

    using BaseContext::EndDict;
    using BaseContext::Key;
  };

  class ArrayItemContext : public BaseContext {
   public:
    ArrayItemContext(Builder& builder) : BaseContext(builder) {}

    KeyItemContext& Key(std::string key) = delete;
    Builder& EndDict() = delete;
    Node Build() = delete;

    ArrayItemContext Value(Node node) {
      return BaseContext::Value(node);
    }
  };

 public:
  // Builder();
  KeyItemContext Key(std::string key);
  Builder& Value(Node node);
  DictItemContext StartDict();
  ArrayItemContext StartArray();
  Builder& EndDict();
  Builder& EndArray();
  Node Build();

 private:
  Node root_;
  std::vector<Node*> nodes_stack_ = {&root_};
  std::optional<std::string> cur_key_;
};

}  // namespace json