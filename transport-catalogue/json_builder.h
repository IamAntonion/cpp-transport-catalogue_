#pragma once
 
#include "json.h"
#include <stack>
#include <string>
#include <memory>
 
namespace transport_catalogue {
namespace detail {
namespace json {
namespace builder {
 
class KeyItemContext;
class DictItemContext;
class ArrayItemContext;

class Builder {
public:
    Builder() = default;
    KeyItemContext Key(std::string key_);
    Builder& Value(Node::Value value_);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();
private:
    Node root_;
    std::vector<std::unique_ptr<Node>> nodes_stack_;
};

// вспомогательный класс, хранит ссылку на Builder
class BaseContext {
public:
    BaseContext(Builder& builder);
    KeyItemContext Key(std::string key);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& Value(Node::Value value);
    Builder& EndDict();
    Builder& EndArray();
protected:
    Builder& builder_;
};

class KeyItemContext : public BaseContext {
public:
    KeyItemContext(Builder& builder);
    KeyItemContext Key(std::string key) = delete;
    BaseContext EndDict() = delete;
    BaseContext EndArray() = delete;
    DictItemContext Value(Node::Value value);
};
 
class DictItemContext : public BaseContext {
public:
    DictItemContext(Builder& builder);
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
    Builder& Value(Node::Value value) = delete;
};
 
class ArrayItemContext : public BaseContext {
public:
    ArrayItemContext(Builder& builder);
    ArrayItemContext Value(Node::Value value);
    KeyItemContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
};
 
} // end namespace builder
} // end namespace json
} // end namespace detail
} // end namespace transport_catalogue