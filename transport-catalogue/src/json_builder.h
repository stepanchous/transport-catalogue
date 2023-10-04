#pragma once

#include <memory>
#include <stack>
#include <variant>

#include "json.h"

namespace json {

class Builder;
class KeyContext;
class DictContext;
class ArrayContext;

template <typename ValueContext>
class BaseContext {
   public:
    BaseContext(Builder& builder);

    ValueContext Value(Node::Value value);
    KeyContext Key(std::string key);

    DictContext StartDict();
    Builder& EndDict();

    ArrayContext StartArray();
    Builder& EndArray();

   private:
    Builder& builder_;
};

class KeyContext : public BaseContext<DictContext> {
   public:
    KeyContext(Builder& builder);
    KeyContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
};

class ProxyValue {
   public:
    ProxyValue(Builder& builder);
};

class DictContext : public BaseContext<ProxyValue> {
   public:
    DictContext(Builder& builder);
    ProxyValue Value(Node::Value value) = delete;
    DictContext StartDict() = delete;
    ArrayContext StartArray() = delete;
    Builder& EndArray() = delete;
};

class ArrayContext : public BaseContext<ArrayContext> {
   public:
    ArrayContext(Builder& builder);
    KeyContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
};

class Builder {
   public:
    Node Build();

    Builder& Value(Node::Value value);

    KeyContext Key(std::string key);

    ArrayContext StartArray();

    Builder& EndArray();

    DictContext StartDict();

    Builder& EndDict();

   private:
    enum class NodeState {
        COMPLETE_OBJ,
        KEY,
        ARRAY_START,
        DICT_START,
    };

    struct HelperNode {
        NodeState state;
        std::unique_ptr<Node> node;
    };

    std::pair<std::string, Node> GetKeyValuePair();

   private:
    std::stack<HelperNode> nodes_;
};

}  // namespace json
