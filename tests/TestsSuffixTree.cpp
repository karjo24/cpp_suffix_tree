#include "../src/SuffixTree.hpp"
#include <queue>
#include <print>
#include <gtest/gtest.h>

using namespace suffixtrees;

TEST(SuffixTreeConstruction, SearchBehaviourTruePositives) {
    std::string s("AAAGCTAGCTATTATTAAAAGCTAGCTAAACGAAGTAGCTATTATACGACGACTAAAAATCGACGACGATTATAAGCGATCAGTCGAAGCTAGT$");
    std::string_view v(s);
    SuffixTree tree(s, true);
    for (auto start = v.begin(); start != v.end(); ++start) {
        for (auto end = start; end != v.end(); ++end) {
            ASSERT_TRUE(tree.search(start, end));
        }
    }
}