#include "json_builder.h"

namespace transport_catalogue {
namespace detail {
namespace json {
namespace builder {

BaseContext::BaseContext(Builder& builder) : builder_(builder) {}
 
    KeyItemContext BaseContext::Key(std::string key) {
        return builder_.Key(key);
    }
    Builder& BaseContext::Value(Node::Value value) {
        return builder_.Value(value);
    }
 
    DictItemContext BaseContext::StartDict() {
        return DictItemContext(builder_.StartDict());
    }
    Builder& BaseContext::EndDict() {
        return builder_.EndDict();
    }
 
    ArrayItemContext BaseContext::StartArray() {
        return ArrayItemContext(builder_.StartArray());
    }
    Builder& BaseContext::EndArray() {
        return builder_.EndArray();
    }
    
    KeyItemContext::KeyItemContext(Builder& builder) : BaseContext(builder) {}
 
    DictItemContext  KeyItemContext::Value(Node::Value value) {
        return BaseContext::Value(std::move(value));
    }
    
    DictItemContext::DictItemContext(Builder& builder) : BaseContext(builder) {}
    
    ArrayItemContext::ArrayItemContext(Builder& builder) : BaseContext(builder) {}
 
    ArrayItemContext ArrayItemContext::Value (Node::Value value) {
        return BaseContext::Value(move(value)); 
    }
    
    KeyItemContext Builder::Key(std::string key_) {
        if (nodes_stack_.empty()) throw std::logic_error("key isn't to create");
        nodes_stack_.emplace_back(std::make_unique<Node>(key_));
        
        return *this;
    }
    
    Builder& Builder::Value(Node::Value value) {
        Node tmp;
        
        tmp.GetValue() = value;

        if (nodes_stack_.empty()) {
            if (!root_.IsNull()) throw std::logic_error("root isn't add");
            root_ = std::move(tmp);
            return *this;
        }

        if (!nodes_stack_.back()->IsArray() && !nodes_stack_.back()->IsString()) 
        throw std::logic_error("unable to create node");
        
        if (nodes_stack_.back()->IsArray()) {
            auto arr = nodes_stack_.back()->AsArray();
            arr.emplace_back(std::move(tmp));

            nodes_stack_.pop_back();
            nodes_stack_.emplace_back(std::move(std::make_unique<Node>(arr)));
        }
        
        if (nodes_stack_.back()->IsString()) {
            std::string key = nodes_stack_.back()->AsString();
            nodes_stack_.pop_back();

            if (nodes_stack_.back()->IsDict()) {
                auto dict = nodes_stack_.back()->AsDict();
                dict.emplace(std::move(key), tmp);

                nodes_stack_.pop_back();
                nodes_stack_.emplace_back(std::move(std::make_unique<Node>(dict)));
            }
        }

        return *this;
    }

    DictItemContext Builder::StartDict() {
        nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Dict())));

        return *this;
    }
    
    ArrayItemContext Builder::StartArray() {
        nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Array())));

        return *this;
    }

    Builder& Builder::EndDict() {
        if (nodes_stack_.empty()) throw std::logic_error("nodes_stack_ is empty in dictionary");
        Node tmp = *nodes_stack_.back();
        
        if (!tmp.IsDict()) throw std::logic_error("object isn't dictionary");
        
        nodes_stack_.pop_back();

        if (nodes_stack_.empty()) {
            if (!root_.IsNull()) throw std::logic_error("root isn't add");
            root_ = std::move(tmp);
            return *this;
        }
        if (!nodes_stack_.back()->IsArray() && !nodes_stack_.back()->IsString()) 
        throw std::logic_error("unable to create node");
        
        if (nodes_stack_.back()->IsArray()) {
            auto arr = nodes_stack_.back()->AsArray();
            arr.emplace_back(std::move(tmp));

            nodes_stack_.pop_back();
            nodes_stack_.emplace_back(std::move(std::make_unique<Node>(arr)));
        }
        
        if (nodes_stack_.back()->IsString()) {
            std::string key = nodes_stack_.back()->AsString();
            nodes_stack_.pop_back();

            if (nodes_stack_.back()->IsDict()) {
                auto dict = nodes_stack_.back()->AsDict();
                dict.emplace(std::move(key), tmp);

                nodes_stack_.pop_back();
                nodes_stack_.emplace_back(std::move(std::make_unique<Node>(dict)));
            }
        }
        return *this;
    }
    
    Builder& Builder::EndArray() {
        if (nodes_stack_.empty()) throw std::logic_error("nodes_stack_ is empty in array");
        Node tmp = *nodes_stack_.back();
        
        if (!tmp.IsArray()) throw std::logic_error("object isn't array");
        nodes_stack_.pop_back();

        if (nodes_stack_.empty()) {
            if (!root_.IsNull()) throw std::logic_error("root isn't add");
            root_ = std::move(tmp);
            return *this;
        }
        
        if (!nodes_stack_.back()->IsArray() && !nodes_stack_.back()->IsString()) 
        throw std::logic_error("unable to create node");
        
        if (nodes_stack_.back()->IsArray()) {
            auto arr = nodes_stack_.back()->AsArray();
            arr.emplace_back(std::move(tmp));

            nodes_stack_.pop_back();
            nodes_stack_.emplace_back(std::move(std::make_unique<Node>(arr)));
        }
        
        if (nodes_stack_.back()->IsString()) {
            std::string key = nodes_stack_.back()->AsString();
            nodes_stack_.pop_back();

            if (nodes_stack_.back()->IsDict()) {
                auto dict = nodes_stack_.back()->AsDict();
                dict.emplace(std::move(key), tmp);

                nodes_stack_.pop_back();
                nodes_stack_.emplace_back(std::move(std::make_unique<Node>(dict)));
            }
        }
        return *this;
    }
    
    Node Builder::Build() {
        if (root_.IsNull()) throw std::logic_error("json is empty");
        if (!nodes_stack_.empty()) throw std::logic_error("json invalid to open");
        return root_;
    }
} // end namespace builder
} // end namespace json
} // end namespace detail
} // end namespace transport_catalogue