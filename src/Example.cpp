//
// Created by johan on 06.01.26.
//
#include <print>

#include "SuffixTree.hpp"

int main() {
    std::string s = "GAGCTAG$";
    suffixtrees::SuffixTree tree(s);

    std::string inText = "GCT";
    std::string notInText = "AAG";
    auto isFound1 = tree.search(inText.begin(), inText.end());
    auto isFound2 = tree.search(notInText.begin(), notInText.end());

    std::print("Is string {} in text? {}", inText, isFound1.first);
    if (isFound1.first) std::println(", at position {}", isFound1.second);
    else std::println("");
    std::print("Is string {} in text? {}", notInText, isFound2.first);
    if (isFound2.first) std::println(", at position {}", isFound2.second);
    else std::println("");

    return 0;
}