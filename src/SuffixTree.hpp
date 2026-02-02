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
     * @param start_node Node which to start descent from
     * @return first value: pointer to closest inner node above end of tree descent\n
     *      second value: nullptr if descent ends exactly on inner node, else: pointer to closest node below end of tree descent\n
     *      third value: 0 if descent ends exactly on inner node, else: position of end of descent relative to edge label
    */
    template <typename It>
    std::tuple<Node *, Node *, std::size_t> skipWalkDown(It start,
        const std::size_t skipCount,
        Node *start_node) {
        Node *curr_node = start_node;

        std::size_t advance;
        for (std::size_t i = 0; i < skipCount; i += advance) {
            Node *child = curr_node->getChild(*start);
            advance = std::min(skipCount - i, child->label.length());
            if (advance < child->label.length()) {
                return std::make_tuple(curr_node, child, advance);
            }
            std::advance(start, advance);
            curr_node = child;
        }
        return std::make_tuple(curr_node, nullptr, 0);
    }

    /**
     * Constructs the suffix tree using the Ukkonen algorithm
     */
    void ukkonenConstruction() {
        assert(!str_view.empty());
        Node *curr_node = root.get(), *child = root.get(), *suffix_link_pending = nullptr;
        std::size_t posOnEdge = 0;
        std::size_t jInitializer = 0;
        bool moveForward = false;

        for (std::size_t i = 0; i < str_view.length(); ++i) {
            // Phase i
            auto prefix_end_it = str_view.begin() + i;

            for (std::size_t j = jInitializer; j <= i; ++j) {
                std::string_view &gamma = child->label;

                auto start = gamma.begin();
                std::size_t skipCount = posOnEdge;
                Node *startNode = curr_node;

                if (moveForward) {
                    startNode = curr_node->suffixLink;
                    if (curr_node == root.get()) {
                        if (skipCount > 0) --skipCount; // TODO this should never be triggered if skipCount is 0; test that it actually doesn't (except in first iteration?)
                        ++start;
                    }
                }

                auto walkDownInformation = skipWalkDown(start, skipCount, startNode); // basically step 2 of SEA

                std::tie(curr_node, child, posOnEdge) = walkDownInformation;

                if (!child) {
                    if (suffix_link_pending) suffix_link_pending->suffixLink = curr_node;

                    if (curr_node->getChild(*prefix_end_it)) {
                        // Rule 3: String implicitly in tree, break
                        ++posOnEdge;
                        child = curr_node->getChild(*prefix_end_it);
                        moveForward = false;
                        suffix_link_pending = nullptr;
                        break;
                    } else {
                        // Rule 2: Split by creating new child node
                        ++jInitializer;
                        moveForward = true;
                        child = &curr_node->constructChild(*prefix_end_it,
                            std::string_view(prefix_end_it, str_view.end()), j);
                        suffix_link_pending = nullptr;
                    }
                } else {
                    if (child->label[posOnEdge] == *prefix_end_it) {
                        // Rule 3: String implicitly in tree, break
                        posOnEdge = (posOnEdge + 1) % child->label.length(); // TODO update curr_node???
                        if (posOnEdge == 0) {
                            curr_node = child;
                        }
                        moveForward = false;
                        suffix_link_pending = nullptr;
                        break;
                    }
                    // Rule 2: Split by splitting an edge
                    ++jInitializer;
                    moveForward = true;
                    char oldChildIndex = *child->label.begin();
                    std::unique_ptr<NodeT> oldChild = curr_node->extractChild(oldChildIndex);
                    // Create new node at split and new leaf as its child
                    Node &newNode = curr_node->
                        constructChild(oldChildIndex, oldChild->label.substr(0, posOnEdge));
                    newNode.constructChild(*prefix_end_it, std::string_view(prefix_end_it, str_view.end()), j);
                    oldChild->trimLabeltoSuffix(posOnEdge);

                    // Move old child to child of newly created node
                    newNode.insertChild(*oldChild->label.begin(), std::move(oldChild));
                    if (suffix_link_pending) suffix_link_pending->suffixLink = &newNode;
                    // Will get its suffix link in phase j+1
                    suffix_link_pending = &newNode;
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