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
    bool isFound1 = tree.search(inText.begin(), inText.end());
    bool isFound2 = tree.search(notInText.begin(), notInText.end());
    std::println("Is string {} in text? {}", inText, isFound1);
    std::println("Is string {} in text? {}", notInText, isFound2);

    return 0;
}