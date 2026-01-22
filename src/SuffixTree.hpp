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
        root = std::make_unique<NodeT>(std::string_view("")); // empty label for root

        for (auto str_it = str_view.begin(); str_it != str_view.end(); ++str_it) {
            auto suffix_it = str_it;

            Node *curr_node, *child;
            decltype(std::mismatch(curr_node->getLabel().begin(), curr_node->getLabel().end(), suffix_it)) pair;
            for (curr_node = root.get(), child = curr_node->getChild(*str_it); child != nullptr;
                 curr_node = child, child = curr_node->getChild(*suffix_it)) {
                pair = std::mismatch(child->getLabel().begin(), child->getLabel().end(), suffix_it);
                suffix_it += std::distance(child->getLabel().begin(), pair.first);
                if (pair.first != child->getLabel().end()) {
                    break;
                }
            }

            // if split: determine split position, update child label, construct new node, move old child into new node child, leave if block with curr_child pointing to newly created split node
            if (child != nullptr && pair.first != child->getLabel().end()) {
                std::size_t splitPos = std::distance(child->getLabel().begin(), pair.first);
                char childIndex = *child->getLabel().begin();
                std::unique_ptr<NodeT> oldChild = curr_node->extractChild(childIndex);
                oldChild->trimLabeltoSuffix(splitPos);
                Node &newNode = curr_node->
                    constructChild(childIndex, std::string_view(suffix_it - splitPos, suffix_it));

                newNode.insertChild(*oldChild->getLabel().begin(), std::move(oldChild));
                curr_node = &newNode;
            }
            // regardless of split: construct new node as child of curr_node with label of [suffix_it,...]
            curr_node->constructChild(*suffix_it, std::string_view(suffix_it, str_view.end()));
        }
    }

    explicit SuffixTree(const StringT &string) : str(string) {
        str_view = std::string_view(str);
        root = std::make_unique<NodeT>(std::string_view("")); // empty label for root

        for (auto str_it = str_view.begin(); str_it != str_view.end(); ++str_it) {
            auto suffix_it = str_it;

            Node *curr_node, *child;
            decltype(std::mismatch(curr_node->getLabel().begin(), curr_node->getLabel().end(), suffix_it)) pair;
            for (curr_node = root.get(), child = curr_node->getChild(*str_it); child != nullptr;
                 curr_node = child, child = curr_node->getChild(*suffix_it)) {
                pair = std::mismatch(child->getLabel().begin(), child->getLabel().end(), suffix_it);
                suffix_it += std::distance(child->getLabel().begin(), pair.first);
                if (pair.first != child->getLabel().end()) {
                    break;
                }
            }

            // if split: determine split position, update child label, construct new node, move old child into new node child, leave if block with curr_child pointing to newly created split node
            if (child != nullptr && pair.first != child->getLabel().end()) {
                std::size_t splitPos = std::distance(child->getLabel().begin(), pair.first);
                char childIndex = *child->getLabel().begin();
                std::unique_ptr<NodeT> oldChild = curr_node->extractChild(childIndex);
                oldChild->trimLabeltoSuffix(splitPos);
                Node &newNode = curr_node->
                    constructChild(childIndex, std::string_view(suffix_it - splitPos, suffix_it));

                newNode.insertChild(*oldChild->getLabel().begin(), std::move(oldChild));
                curr_node = &newNode;
            }
            // regardless of split: construct new node as child of curr_node with label of [suffix_it,...]
            curr_node->constructChild(*suffix_it, std::string_view(suffix_it, str_view.end()));
        }
    }


    SuffixTree(const SuffixTree &) = delete;
    SuffixTree &operator=(const SuffixTree &) = delete;

    SuffixTree(SuffixTree &&) = delete;

    SuffixTree &operator=(SuffixTree &&) = delete;
};
} // suffixtrees