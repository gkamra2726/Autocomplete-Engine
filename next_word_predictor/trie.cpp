#include "trie.hpp"
#include <algorithm>

Trie::Trie() {
    root = new TrieNode();
    totalWords = 0;
}

Trie::~Trie() {
    // Note: for a class project, we skip recursive node deletion for brevity.
    // In production code you'd write a recursive destructor here.
}

int Trie::charIndex(char c) const {
    if (c >= 'a' && c <= 'z') return c - 'a';
    return -1;
}

void Trie::insert(const std::string& word) {
    TrieNode* curr = root;
    bool isNewWord = false;

    for (char c : word) {
        int idx = charIndex(c);
        if (idx == -1) continue; // skip any stray non-lowercase char safely

        if (curr->children[idx] == nullptr) {
            curr->children[idx] = new TrieNode();
        }
        curr = curr->children[idx];
    }

    if (!curr->isEndOfWord) {
        isNewWord = true;
    }
    curr->isEndOfWord = true;
    curr->frequency++;

    if (isNewWord) totalWords++;
}

bool Trie::search(const std::string& word) const {
    TrieNode* node = findNode(word);
    return node != nullptr && node->isEndOfWord;
}

TrieNode* Trie::findNode(const std::string& prefix) const {
    TrieNode* curr = root;
    for (char c : prefix) {
        int idx = charIndex(c);
        if (idx == -1 || curr->children[idx] == nullptr) {
            return nullptr; // prefix doesn't exist in trie
        }
        curr = curr->children[idx];
    }
    return curr;
}

void Trie::collectWords(TrieNode* node, std::string wordSoFar,
                         std::vector<std::pair<std::string,int>>& results) const {
    if (node == nullptr) return;

    if (node->isEndOfWord) {
        results.push_back({wordSoFar, node->frequency});
    }

    for (int i = 0; i < 26; i++) {
        if (node->children[i] != nullptr) {
            collectWords(node->children[i], wordSoFar + char('a' + i), results);
        }
    }
}

std::vector<std::string> Trie::autocomplete(const std::string& prefix, int k) const {
    std::vector<std::string> suggestions;

    TrieNode* prefixNode = findNode(prefix);
    if (prefixNode == nullptr) return suggestions; // no words match this prefix

    std::vector<std::pair<std::string,int>> matches;
    collectWords(prefixNode, prefix, matches);

    // Sort by frequency descending, then alphabetically as tiebreaker
    std::sort(matches.begin(), matches.end(),
        [](const std::pair<std::string,int>& a, const std::pair<std::string,int>& b) {
            if (a.second != b.second) return a.second > b.second;
            return a.first < b.first;
        });

    for (int i = 0; i < (int)matches.size() && i < k; i++) {
        suggestions.push_back(matches[i].first);
    }

    return suggestions;
}
