//
// Created by johan on 07.01.26.
//
#pragma once
#include <array>
#include <memory>
#include <unordered_map>

namespace suffixtrees {
class SuffixTree;

class Node {
    friend class SuffixTree;
    std::array<std::unique_ptr<Node>, 5> children;
    std::string_view label;
    Node *suffixLink;
    const std::size_t suffixIndex = 0;

public:
    explicit Node(const std::string_view str_view) : label(str_view), suffixLink(this) {}
    Node(const std::string_view str_view, const std::size_t suffixIndex) : label(str_view), suffixLink(this), suffixIndex(suffixIndex) {}

    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;
    Node(Node &&other) = delete;
    Node &operator=(Node &&) = delete;

    template <typename... Args>
    Node &constructChild(const char c, Args... args) {
        const std::size_t idx = _charMap[c];
        children[idx] = std::make_unique<Node>(std::forward<Args>(args)...);
        return *children[idx];
    }

    Node *getChild(const char c) const {
        return children[_charMap[c]].get();
    }

    std::unique_ptr<Node> extractChild(const char c) {
        return std::move(children[_charMap[c]]);
    }

    void insertChild(const char c, std::unique_ptr<Node> &&child) {
        children[_charMap[c]] = std::move(child);
    }

    void trimLabeltoSuffix(std::size_t start) {
        label = label.substr(start);
    }

private:
    static inline std::unordered_map<char, std::size_t> _charMap = {{'A', 0}, {'T', 1}, {'G', 2}, {'C', 3}, {'$', 4}};
};
} // suffixtrees