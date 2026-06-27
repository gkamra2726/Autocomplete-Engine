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
// Helper function jo trie branch me recursive traverse karega aur row update karega
// 1. searchEditDistance Function (Aapke children[26] array ke liye perfectly modified)
void Trie::searchEditDistance(TrieNode* node, char ch, const std::string& word, 
                               const std::vector<int>& previousRow, const std::string& target, 
                               std::vector<std::pair<std::string, int>>& results, int maxCost) const {
    int columns = target.length() + 1;
    std::vector<int> currentRow(columns);
    currentRow[0] = previousRow[0] + 1;

    // Levenshtein Edit Distance Table dynamic calculation
    for (int i = 1; i < columns; i++) {
        int insertCost = currentRow[i - 1] + 1;
        int deleteCost = previousRow[i] + 1;
        int replaceCost = (target[i - 1] == ch) ? previousRow[i - 1] : previousRow[i - 1] + 1;

        currentRow[i] = std::min({insertCost, deleteCost, replaceCost});
    }

    // Agar word Trie me end ho raha hai aur typo range ke andar hai
    if (currentRow[columns - 1] <= maxCost && node->isEndOfWord) {
        results.push_back({word, currentRow[columns - 1]});
    }

    // Optimization: Agar current row ka minimum value bhi maxCost se bada hai, toh aage nahi jayenge
    int minRowVal = *std::min_element(currentRow.begin(), currentRow.end());
    if (minRowVal <= maxCost) {
        // YAHAN FIX HAI: Array structure ke 26 slots par loop chalega
        for (int i = 0; i < 26; i++) {
            if (node->children[i] != nullptr) {
                char nextChar = 'a' + i; // Index ko wapas character me badla
                searchEditDistance(node->children[i], nextChar, word + nextChar, currentRow, target, results, maxCost);
            }
        }
    }
}

// 2. autocompleteWithEditDistance Function
std::vector<std::string> Trie::autocompleteWithEditDistance(const std::string& word, int maxCost, int k) const {
    std::vector<std::pair<std::string, int>> distanceResults;
    int columns = word.length() + 1;
    std::vector<int> firstRow(columns);
    
    for (int i = 0; i < columns; i++) {
        firstRow[i] = i;
    }

    // Root node ke 26 children array check kiye ja rahe hain
    for (int i = 0; i < 26; i++) {
        if (root->children[i] != nullptr) {
            char nextChar = 'a' + i;
            std::string currentWord = "";
            currentWord += nextChar;
            searchEditDistance(root->children[i], nextChar, currentWord, firstRow, word, distanceResults, maxCost);
        }
    }

    // Sorting results: Kam edit distance (mistakes) pehle aayegi
    std::sort(distanceResults.begin(), distanceResults.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) return a.second < b.second; // Lower cost wins
        return a.first < b.first; // Alphabetical order fallback
    });

    std::vector<std::string> finalSuggestions;
    for (size_t i = 0; i < distanceResults.size() && (int)i < k; i++) {
        finalSuggestions.push_back(distanceResults[i].first);
    }
    return finalSuggestions;
}
