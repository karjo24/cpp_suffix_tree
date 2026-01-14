//
// Created by johan on 07.01.26.
//
#pragma once
#include <array>
#include <memory>
#include <unordered_map>

namespace suffixtrees {
    class Node {
        std::array<std::unique_ptr<Node>, 5> children;
        struct Label {
            const std::string_view& str_view;
            std::size_t start, end;
            Label(const std::string_view& view, std::size_t start, std::size_t end) : str_view(view), start(start), end(end) {}
        };
        Label label;

    public:
        Node(const std::string_view& str_view, std::size_t start, std::size_t end) : label(str_view, start, end) {};

        Node(const Node &) = delete;
        Node &operator=(const Node &) = delete;
        Node(Node && other) = delete;
        Node &operator=(Node &&) = delete;
    };
} // suffixtrees