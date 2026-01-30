#pragma once

#include "Node.hpp"
#include <cassert>
#include <memory>

namespace suffixtrees {
class SuffixTree {
    using NodeT = Node;
    using StringT = std::string;
    using CharT = std::string::value_type;

    StringT str;
    std::string_view str_view;
    std::unique_ptr<NodeT> root;

    void naiveConstruction() {
        for (auto str_it = str_view.begin(); str_it != str_view.end(); ++str_it) {
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
            curr_node->constructChild(*suffix_it, std::string_view(suffix_it, str_view.end()));
        }
    }

    /**
     * Walks down sequence of skipCount chars after start below start_node; Assumes this sequence is present below start_node
     * @tparam It Iterator type
     * @param start Iterator to walk down along
     * @param skipCount Skip skipCount chars
     * @param start_node Node which to start descent from
     * @param distanceFromEnd True leaf length in current phase must be leaf.length() - distanceFromEnd
     * @return first value: pointer to closest inner node above end of tree descent\n
     *      second value: nullptr if descent ends exactly on inner node, else: pointer to closest node below end of tree descent\n
     *      third value: 0 if descent ends exactly on inner node, else: position of end of descent relative to edge label
    */
    template <typename It>
    std::tuple<Node *, Node *, std::size_t> skipWalkDown(It start,
        const std::size_t skipCount,
        Node *start_node,
        const std::size_t distanceFromEnd) {
        Node *curr_node = start_node;

        std::size_t advance;
        for (std::size_t i = 0; i < skipCount; i += advance) {
            Node *child = curr_node->getChild(*start);
            // Optimization: Leaf labels are complete at any point in the algorithm to avoid explicit extensions;
            // Tradeoff: label.length() doesn't reflect true length in case of leafs
            const std::size_t labelLength = child->label.length();
            std::size_t trueLabelLength =
                child->label.end() != str_view.end() ? labelLength : labelLength - distanceFromEnd;
            advance = std::min(skipCount - i, trueLabelLength);
            if (advance < trueLabelLength || child->label.end() == str_view.end()) {
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
        root->constructChild(*str_view.begin(), str_view);

        auto start = str_view.begin();
        for (auto prefix_end_it = ++start; prefix_end_it != str_view.end(); ++prefix_end_it) {
            // Phase i
            Node *curr_node = root.get(), *child = root->getChild(*str_view.begin()), *suffix_link_pending = nullptr;
            const std::size_t distanceFromEnd = std::distance(prefix_end_it, str_view.end());
            std::size_t posOnEdge = str_view.length() - distanceFromEnd;

            // Sketchy, but okay: suffix_of_prefix_it one-past-the-end-iterator at maximum and never dereferenced, TODO
            for (std::size_t j = 0; j < str_view.length(); ++j) {
                /** Phase j
                 *  At the start of each phase j, the variables represent the following:
                 *      prefix_end_it: Iterator to the T[i+1], which is used for each extension
                 *      curr_node: The closest (inner) node above the end point of the previous iteration already containing a suffix link
                 *      child: The closest node below the end point of the previous iteration
                 *      posOnEdge: End position of j-1 iteration relative to label if j > 0; Absolute position of end of prefix of whole string if j == 0
                 *      distanceFromEnd: current "true" leaf label length of each leaf is label.length() - distanceFromEnd; Context: Leaf labels are constructed containing a whole suffix to avoid explicit leaf extensions.
                 */

                std::string_view &gamma = child->label;

                auto start = gamma.begin();
                std::size_t skipCount = posOnEdge;
                if (curr_node == root.get() && j != 0) {
                    ++start;
                    if (skipCount > 0) --skipCount;
                }

                auto walkDownInformation = skipWalkDown(start, skipCount, curr_node->suffixLink,
                    distanceFromEnd); // basically step 2 of SEA

                std::tie(curr_node, child, posOnEdge) = walkDownInformation;

                if (!child) {
                    if (suffix_link_pending) suffix_link_pending->suffixLink = curr_node;

                    if (curr_node->getChild(*prefix_end_it)) {
                        // Rule 3: String implicitly in tree, break
                        break;
                    } else {
                        // Rule 2: Split by creating new child node
                        child = &curr_node->constructChild(*prefix_end_it,
                            std::string_view(prefix_end_it, str_view.end()));
                        suffix_link_pending = nullptr;
                    }
                } else if (child->label.end() == str_view.end() && (child->label.length() - posOnEdge) ==
                           distanceFromEnd) {
                    // Rule 1: Extend leaf
                    // Implicit extension: Leafs are always created with the label reaching the end
                    suffix_link_pending = nullptr;
                } else {
                    if (child->label[posOnEdge] == *prefix_end_it) {
                        // Rule 3: String implicitly in tree, break
                        break;
                    }
                    // Rule 2: Split by splitting an edge
                    char oldChildIndex = *child->label.begin();
                    std::unique_ptr<NodeT> oldChild = curr_node->extractChild(oldChildIndex);
                    // Create new node at split and new leaf as its child
                    Node &newNode = curr_node->
                        constructChild(oldChildIndex, oldChild->label.substr(0, posOnEdge));
                    newNode.constructChild(*prefix_end_it, std::string_view(prefix_end_it, str_view.end()));
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
        }
        return true;
    }
};
} // suffixtrees