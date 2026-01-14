#pragma once

#include "Node.hpp"
#include <memory>

namespace suffixtrees {
class SuffixTree {
    using NodeT = Node;
    using StringT = std::string;
    using CharT = std::string::value_type;

    StringT str;
    std::string_view str_view;
    std::unique_ptr<NodeT> root;

public:
    explicit SuffixTree(StringT &&string) : str(std::move(string)) {
        str_view = std::string_view(str);
        root = std::make_unique<NodeT>(str_view, 0, 0);
    };

    explicit SuffixTree(const StringT &string) : str(string) {
        str_view = std::string_view(str);
        root = std::make_unique<NodeT>(str_view, 0, 0);

        for (auto it = str_view.rbegin(); it != str_view.rend(); ++it) {}
    };

    SuffixTree(const SuffixTree &) = delete;
    SuffixTree &operator=(const SuffixTree &) = delete;

    SuffixTree(SuffixTree &&) = delete;

    SuffixTree &operator=(SuffixTree &&) = delete;
};
} // suffixtrees