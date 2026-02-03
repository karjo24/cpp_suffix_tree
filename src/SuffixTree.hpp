#pragma once

#include "Node.hpp"
#include <cassert>
#include <memory>

namespace suffixtrees {
class SuffixTree {
    friend class SuffixTreeConstruction_ConstructionAlgorithmEquality_Test;
    using NodeT = Node;
    using StringT = std::string;
    using CharT = std::string::value_type;

    StringT str;
    std::string_view str_view;
    std::unique_ptr<NodeT> root;

    void naiveConstruction() {
        std::size_t j = 0;
        for (auto str_it = str_view.begin(); str_it != str_view.end(); ++str_it, ++j) {
            auto suffix_it = str_it;

            Node *curr_node, *child;
            decltype(std::mismatch(curr_node->label.begin(), curr_node->label.end(), suffix_it)) pair;
            for (curr_node = root.get(), child = curr_node->getChild(*str_it); child != nullptr;
                 curr_node = child, child = curr_node->getChild(*suffix_it)) {
                pair = std::mismatch(child->label.begin(), child->label.end(), suffix_it);
                suffix_it += std::distance(child->label.begin(), pair.first);
                if (pair.first != child->label.end()) {
                    break;
                }
            }

            // if split: determine split position, update child label, construct new node, move old child into new node child, leave if block with curr_child pointing to newly created split node
            if (child != nullptr && pair.first != child->label.end()) {
                std::size_t splitPos = std::distance(child->label.begin(), pair.first);
                char childIndex = *child->label.begin();
                std::unique_ptr<NodeT> oldChild = curr_node->extractChild(childIndex);
                oldChild->trimLabeltoSuffix(splitPos);
                Node &newNode = curr_node->
                    constructChild(childIndex, std::string_view(suffix_it - splitPos, suffix_it));

                newNode.insertChild(*oldChild->label.begin(), std::move(oldChild));
                curr_node = &newNode;
            }
            // regardless of split: construct new node as child of curr_node with label of [suffix_it,...]
            curr_node->constructChild(*suffix_it, std::string_view(suffix_it, str_view.end()), j);
        }
    }

    /**
     * Walks down sequence of skipCount chars after start below start_node; Assumes this sequence is present below start_node
     * @tparam It Iterator type
     * @param start Iterator to walk down along
     * @param skipCount Skip skipCount chars
     * @param startNode Node which to start descent from
     * @return first value: pointer to closest inner node above end of tree descent\n
     *      second value: nullptr if descent ends exactly on inner node, else: pointer to closest node below end of tree descent\n
     *      third value: 0 if descent ends exactly on inner node, else: position of end of descent relative to edge label
    */
    template <typename It>
    std::tuple<Node *, Node *, std::size_t> skipWalkDown(It start,
        const std::size_t skipCount,
        Node *startNode) {
        Node *currNode = startNode;

        std::size_t advance;
        for (std::size_t i = 0; i < skipCount; i += advance) {
            Node *child = currNode->getChild(*start);
            advance = std::min(skipCount - i, child->label.length());
            if (advance < child->label.length()) {
                return std::make_tuple(currNode, child, advance);
            }
            std::advance(start, advance);
            currNode = child;
        }
        return std::make_tuple(currNode, nullptr, 0);
    }

    /**
     * Constructs the suffix tree using the Ukkonen algorithm
     */
    void ukkonenConstruction() {
        assert(!str_view.empty());
        Node *activeNode = root.get(), *child = root.get(), *suffixLinkPending = nullptr;
        // skipCount represents the position of the
        std::size_t skipCount = 0, jDash = 0;
        bool moveForward = false;

        // Iterator pointing to char which to extend with, increase in i-loop
        auto prefixEndIt = str_view.begin();
        for (std::size_t i = 0; i < str_view.length(); ++i, ++prefixEndIt) {
            // Phase i
            for (std::size_t j = jDash; j <= i; ++j) {
                // Phase j

                // Start skipping characters at suffix link if present or in root
                // Iterator to sequence we went up along to closest inner node
                auto skipLabelIt = child->label.begin();
                Node *startNode = activeNode;
                if (moveForward) {
                    startNode = activeNode->suffixLink;
                    if (activeNode == root.get()) {
                        skipCount = skipCount > 0 ? --skipCount : 0;
                        ++skipLabelIt;
                    }
                }

                std::tie(activeNode, child, skipCount) = skipWalkDown(skipLabelIt, skipCount, startNode);
                child = !child ? activeNode->getChild(*prefixEndIt) : child;

                if (child && child->label[skipCount] == *prefixEndIt) {
                    // Rule 3: String implicitly in tree, break
                    if (suffixLinkPending) suffixLinkPending->suffixLink = activeNode;
                    suffixLinkPending = nullptr;
                    // Do not increase j
                    moveForward = false;
                    // Skip one more char in next iteration (i is increased)
                    skipCount = (skipCount + 1) % child->label.length();
                    if (skipCount == 0) activeNode = child;
                    break;
                } else {
                    // Rule 2
                    ++jDash;
                    moveForward = true;
                    if (!child) {
                        // Rule 2 case 1: Split by creating new child node
                        if (suffixLinkPending) suffixLinkPending->suffixLink = activeNode;
                        child = &activeNode->constructChild(*prefixEndIt,
                            std::string_view(prefixEndIt, str_view.end()), j);
                        suffixLinkPending = nullptr;
                        continue;
                    }

                    // Rule 2 case 2: Split by splitting an edge
                    char oldChildIndex = *child->label.begin();
                    std::unique_ptr<NodeT> oldChild = activeNode->extractChild(oldChildIndex);
                    // Create new node at split, new leaf and move oldChild
                    Node &newNode = activeNode->
                        constructChild(oldChildIndex, oldChild->label.substr(0, skipCount));
                    newNode.constructChild(*prefixEndIt, std::string_view(prefixEndIt, str_view.end()), j);
                    oldChild->trimLabeltoSuffix(skipCount);
                    newNode.insertChild(*oldChild->label.begin(), std::move(oldChild));

                    // Set suffix link and mark newNode as in need of suffix link
                    if (suffixLinkPending) suffixLinkPending->suffixLink = &newNode;
                    suffixLinkPending = &newNode;
                    // Update child from oldChild to newNode
                    // Leave curr_node as is to use its suffix link in next iteration.
                    child = &newNode;
                }
            }
        }
    }

public:
    explicit SuffixTree(StringT &&string, bool useUkkonen = true) : str(std::move(string)) {
        str_view = std::string_view(str);
        root = std::make_unique<NodeT>(std::string_view("")); // empty label for root
        if (useUkkonen) ukkonenConstruction();
        else naiveConstruction();
    }

    explicit SuffixTree(const StringT &string, bool useUkkonen = true) : str(string) {
        str_view = std::string_view(str);
        root = std::make_unique<NodeT>(std::string_view("")); // empty label for root
        if (useUkkonen) ukkonenConstruction();
        else naiveConstruction();
    }


    SuffixTree(const SuffixTree &) = delete;
    SuffixTree &operator=(const SuffixTree &) = delete;

    SuffixTree(SuffixTree &&) = delete;

    SuffixTree &operator=(SuffixTree &&) = delete;

    /**
 * Search for occurrence of sequence [first, last) in string that is owned by this suffix tree
 * @tparam It Iterator type of the view
 * @param first Iterator to start of sequence to search for
 * @param last Iterator to end of sequence [first,last)
 * @return bool indicating if the sequence was found in text
 */
    template <typename It>
    bool search(It first, It last) {
        Node *currentNode = root.get();
        std::size_t totalDistance = 0;
        std::pair pair = std::make_pair(first, str_view.begin());
        while (first != last) {
            if (!Node::_charMap.contains(*first)) return false;
            // TODO refactor into sth more elegant once child container is refactored
            Node *child = currentNode->getChild(*first);
            if (child == nullptr) return false;
            pair = std::mismatch(first, last, child->label.begin(), child->label.end());
            if (pair.first != last && pair.second != child->label.end()) return false;
            totalDistance += std::distance(first, pair.first);
            first = pair.first;
            currentNode = child;
        }
        return true;
    }
};
} // suffixtrees