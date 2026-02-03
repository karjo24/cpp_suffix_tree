#include "../src/SuffixTree.hpp"
#include <random>

#include <gtest/gtest.h>

using namespace suffixtrees;

TEST(SuffixTreeBehaviour, SearchBehaviourTruePositives) {
    std::string s("AAAGCTAGCTATTATTAAAAGCTAGCTAAACGAAGTAGCTATTATACGACGACTAAAAATCGACGACGATTATAAGCGATCAGTCGAAGCTAGT$");
    std::string_view v(s);
    SuffixTree tree(s, true);
    for (auto start = v.begin(); start != v.end(); ++start) {
        for (auto end = start; end != v.end(); ++end) {
            ASSERT_TRUE(tree.search(start, end));
        }
    }
}

// Define useful functions ofr fuzz tests
auto getRandomChar = [](const std::string &charPool, auto &dist, auto &gen) {
    return charPool[dist(gen)];
};
auto createRandomString = [](std::string &str, const std::string &charPool, auto &dist, auto &gen) {
    std::generate(str.begin(), str.end(), [&]() { return getRandomChar(charPool, dist, gen); });
};
auto testSearchEquality = [](const SuffixTree &tree,
    const std::string_view &haystackView,
    const std::string_view &patternView) {
    bool stdLibrarySearch = std::ranges::search(haystackView, patternView).begin() != haystackView.end();
    bool suffixTreeSearch = tree.search(patternView.begin(), patternView.end());
    return stdLibrarySearch == suffixTreeSearch;
};

TEST(SuffixTreeBehaviour, SearchBehaviourFuzz) {
    const std::string chars = "ATGC";
    const std::array NEEDLE_SIZES{3, 5, 7, 10, 15};
    std::mt19937 rng{42};
    std::uniform_int_distribution<std::size_t> uniformDistribution(0, chars.length() - 1);
    std::discrete_distribution<std::size_t> discreteDistribution1({30, 30, 20, 20});
    std::discrete_distribution<std::size_t> discreteDistribution2({10, 10, 40, 40});
    std::discrete_distribution<std::size_t> discreteDistribution3({40, 30, 20, 10});

    const std::size_t HAYSTACK_ITERATIONS = 50;
    for (std::size_t i = 0; i < HAYSTACK_ITERATIONS; ++i) {
        std::string haystack{};
        haystack.reserve(101);
        haystack.resize(100);
        createRandomString(haystack, chars, uniformDistribution, rng);
        haystack += "$";
        std::string_view haystackView(haystack);
        SuffixTree tree(haystack, true);

        for (auto s : NEEDLE_SIZES) {
            std::string needle{};
            needle.resize(s);
            const std::size_t NEEDLE_ITERATIONS = s * 100;
            for (std::size_t j = 0; j < NEEDLE_ITERATIONS; ++j) {
                createRandomString(needle, chars, uniformDistribution, rng);
                ASSERT_TRUE(testSearchEquality(tree, haystackView, std::string_view(needle)));
                createRandomString(needle, chars, discreteDistribution1, rng);
                ASSERT_TRUE(testSearchEquality(tree, haystackView, std::string_view(needle)));
                createRandomString(needle, chars, discreteDistribution2, rng);
                ASSERT_TRUE(testSearchEquality(tree, haystackView, std::string_view(needle)));
                createRandomString(needle, chars, discreteDistribution3, rng);
                ASSERT_TRUE(testSearchEquality(tree, haystackView, std::string_view(needle)));
            }
        }
    }
}