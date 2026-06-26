#ifndef TRIE_HPP
#define TRIE_HPP

#include <string>
#include <vector>

// ---------------------------------------------------------
// TrieNode: classic 26-ary trie node for lowercase a-z words
// ---------------------------------------------------------
struct TrieNode {
    TrieNode* children[26];
    bool isEndOfWord;
    int frequency;   // how many times this exact word appeared in corpus

    TrieNode() {
        isEndOfWord = false;
        frequency = 0;
        for (int i = 0; i < 26; i++) children[i] = nullptr;
    }
};

// ---------------------------------------------------------
// Trie: insert words, search, and prefix-based autocomplete
// ---------------------------------------------------------
class Trie {
private:
    TrieNode* root;

    // Helper: returns char index 0-25, or -1 if not a lowercase letter
    int charIndex(char c) const;

    // DFS from a node, collecting all complete words below it
    // (wordSoFar is the prefix path taken to reach `node`)
    void collectWords(TrieNode* node, std::string wordSoFar,
                       std::vector<std::pair<std::string,int>>& results) const;

public:
    Trie();
    ~Trie();

    void insert(const std::string& word);          // insert/increment frequency
    bool search(const std::string& word) const;     // exact word exists?
    TrieNode* findNode(const std::string& prefix) const; // walk to prefix node

    // Returns top-k completions for a prefix, sorted by frequency (desc)
    std::vector<std::string> autocomplete(const std::string& prefix, int k) const;

    int totalWords;  // unique words inserted (for stats display)
};

#endif
