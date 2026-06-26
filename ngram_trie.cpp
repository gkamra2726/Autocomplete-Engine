#include "ngram_trie.hpp"
#include <algorithm>

NGramTrie::NGramTrie(int maxContextSize) {
    root = new NGramNode();
    maxContext = maxContextSize;
    totalContexts = 0;
}

NGramTrie::~NGramTrie() {
    // Skipping recursive cleanup for brevity, same as Trie.
}

void NGramTrie::insertSequence(const std::vector<std::string>& contextWords,
                                 const std::string& nextWord) {
    NGramNode* curr = root;

    for (const std::string& word : contextWords) {
        if (curr->children.find(word) == curr->children.end()) {
            curr->children[word] = new NGramNode();
            totalContexts++;
        }
        curr = curr->children[word];
    }

    // At this node, record that `nextWord` followed this context
    curr->nextWordFreq[nextWord]++;
}

NGramNode* NGramTrie::findNode(const std::vector<std::string>& contextWords) const {
    NGramNode* curr = root;
    for (const std::string& word : contextWords) {
        auto it = curr->children.find(word);
        if (it == curr->children.end()) return nullptr;
        curr = it->second;
    }
    return curr;
}

void NGramTrie::buildFromCorpus(const std::vector<std::string>& tokens) {
    int n = tokens.size();

    // For every position, and for every context length from 1 up to maxContext,
    // record (preceding `len` words) -> (word at this position).
    // This builds unigram, bigram, trigram... contexts all in the same trie,
    // which is what makes backoff possible later.
    for (int i = 0; i < n; i++) {
        for (int len = 1; len <= maxContext; len++) {
            if (i - len < 0) break; // not enough preceding words for this context length

            std::vector<std::string> context(tokens.begin() + (i - len), tokens.begin() + i);
            insertSequence(context, tokens[i]);
        }
    }
}

std::vector<std::pair<std::string,int>> NGramTrie::predictNext(
        const std::vector<std::string>& contextSoFar, int k) const {

    // Backoff: start with the longest context we have (up to maxContext words),
    // and if there's no match, shrink the context by one word at a time.
    int len = std::min((int)contextSoFar.size(), maxContext);

    for (int tryLen = len; tryLen >= 1; tryLen--) {
        std::vector<std::string> context(
            contextSoFar.end() - tryLen, contextSoFar.end());

        NGramNode* node = findNode(context);
        if (node != nullptr && !node->nextWordFreq.empty()) {
            // Found a matching context with at least one observed next-word.
            std::vector<std::pair<std::string,int>> results(
                node->nextWordFreq.begin(), node->nextWordFreq.end());

            std::sort(results.begin(), results.end(),
                [](const std::pair<std::string,int>& a, const std::pair<std::string,int>& b) {
                    if (a.second != b.second) return a.second > b.second;
                    return a.first < b.first;
                });

            if ((int)results.size() > k) results.resize(k);
            return results;
        }
        // else: shrink context and try again (backoff)
    }

    // No context matched at all (e.g. very first word, or never-seen sequence)
    return {};
}
