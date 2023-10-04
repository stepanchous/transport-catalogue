#include "json_builder.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <variant>

#include "json.h"

namespace json {

using namespace std;

// ------------ Builder ------------

Node Builder::Build() {
    if (nodes_.size() != 1) {
        throw logic_error("Cursed JSON");
    }

    Node root = std::move(*nodes_.top().node);
    nodes_.pop();

    return root;
}

Builder& Builder::Value(Node::Value value) {
    nodes_.push({NodeState::COMPLETE_OBJ, make_unique<Node>(std::move(value))});
    return *this;
}

KeyContext Builder::Key(string key) {
    nodes_.push({NodeState::KEY, make_unique<Node>(std::move(key))});
    return *this;
}

ArrayContext Builder::StartArray() {
    nodes_.push({NodeState::ARRAY_START, nullptr});
    return *this;
}

Builder& Builder::EndArray() {
    auto array_ptr = make_unique<Array>();
    while (!nodes_.empty() && nodes_.top().state != NodeState::ARRAY_START) {
        if (nodes_.top().state != NodeState::COMPLETE_OBJ) {
            throw logic_error("Attempt to add invalid object to array");
        }

        auto node_ptr = std::move(nodes_.top().node);
        nodes_.pop();
        array_ptr->push_back(Node(std::move(*node_ptr)));
    }

    if (nodes_.empty()) {
        throw logic_error("EndArray() call without corresponding StartArray()");
    }

    nodes_.pop();
    reverse(array_ptr->begin(), array_ptr->end());
    nodes_.push(HelperNode{NodeState::COMPLETE_OBJ,
                           make_unique<Node>(std::move(*array_ptr))});

    return *this;
}

DictContext Builder::StartDict() {
    nodes_.push({NodeState::DICT_START, nullptr});
    return *this;
}

Builder& Builder::EndDict() {
    auto dict_ptr = make_unique<Dict>();
    while (!nodes_.empty() && nodes_.top().state != NodeState::DICT_START) {
        if (nodes_.top().state != NodeState::KEY &&
            nodes_.top().state != NodeState::COMPLETE_OBJ) {
            throw logic_error("Attept to add invalid object to dict");
        }
        dict_ptr->insert(GetKeyValuePair());
    }

    if (nodes_.empty()) {
        throw logic_error("EndDict() call without corresponding StartDict()");
    }

    nodes_.pop();
    nodes_.push(HelperNode{NodeState::COMPLETE_OBJ,
                           make_unique<Node>(std::move(*dict_ptr))});

    return *this;
}

pair<string, Node> Builder::GetKeyValuePair() {
    if (nodes_.empty() || nodes_.top().state != NodeState::COMPLETE_OBJ) {
        throw logic_error("Invalid key-value object call chain");
    }

    auto value = std::move(nodes_.top().node);
    nodes_.pop();

    if (nodes_.empty() || nodes_.top().state != NodeState::KEY) {
        throw logic_error("Invalid key-value object call chain");
    }

    auto key = std::move(nodes_.top().node);
    nodes_.pop();

    return {std::move(key->AsString()), std::move(*value)};
}

// ------------ BaseContext ------------
template <typename ValueContext>
BaseContext<ValueContext>::BaseContext(Builder& builder) : builder_(builder) {}

template <typename ValueContext>
ValueContext BaseContext<ValueContext>::Value(Node::Value value) {
    return builder_.Value(std::move(value));
}

template <typename ValueContext>
KeyContext BaseContext<ValueContext>::Key(string key) {
    return builder_.Key(std::move(key));
}

template <typename ValueContext>
DictContext BaseContext<ValueContext>::StartDict() {
    return builder_.StartDict();
}

template <typename ValueContext>
Builder& BaseContext<ValueContext>::EndDict() {
    return builder_.EndDict();
}

template <typename ValueContext>
ArrayContext BaseContext<ValueContext>::StartArray() {
    return builder_.StartArray();
}

template <typename ValueContext>
Builder& BaseContext<ValueContext>::EndArray() {
    return builder_.EndArray();
}

template class BaseContext<DictContext>;
template class BaseContext<ArrayContext>;
template class BaseContext<ProxyValue>;

// ------------ KeyContext ------------
KeyContext::KeyContext(Builder& builder) : BaseContext<DictContext>(builder) {}

// ------------ DictContext ------------
DictContext::DictContext(Builder& builder) : BaseContext<ProxyValue>(builder) {}

// ------------ ArrayContext ------------
ArrayContext::ArrayContext(Builder& builder)
    : BaseContext<ArrayContext>(builder) {}

// ------------ ProxyValue ------------
ProxyValue::ProxyValue([[maybe_unused]] Builder& builder) {}

}  // namespace json
