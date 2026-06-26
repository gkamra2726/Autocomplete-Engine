#ifndef NGRAM_TRIE_HPP
#define NGRAM_TRIE_HPP

#include <string>
#include <vector>
#include <unordered_map>

// ---------------------------------------------------------
// NGramNode: each node = one word in a context sequence.
// nextWordFreq maps "what word follows this context" -> count
// children maps "next context word" -> deeper NGramNode
// ---------------------------------------------------------
struct NGramNode {
    std::unordered_map<std::string, NGramNode*> children;
    std::unordered_map<std::string, int> nextWordFreq;
};

// ---------------------------------------------------------
// NGramTrie: stores context -> next-word frequency mappings.
// Built by sliding a window of size `maxContext` over the corpus.
// Supports backoff: tries longest context first, shortens if no match.
// ---------------------------------------------------------
class NGramTrie {
private:
    NGramNode* root;
    int maxContext; // e.g. 2 means up to trigrams (2 context words + 1 next word)

    // Inserts one (contextWords -> nextWord) observation into the trie
    void insertSequence(const std::vector<std::string>& contextWords,
                         const std::string& nextWord);

    // Walks the trie following contextWords; returns node reached, or nullptr
    NGramNode* findNode(const std::vector<std::string>& contextWords) const;

public:
    NGramTrie(int maxContextSize);
    ~NGramTrie();

    // Feed the full tokenized corpus; builds all n-gram orders from 1..maxContext
    void buildFromCorpus(const std::vector<std::string>& tokens);

    // Predicts top-k next words given the words typed so far.
    // Uses backoff: tries full context, then shorter, then shorter, down to unigram.
    std::vector<std::pair<std::string,int>> predictNext(
        const std::vector<std::string>& contextSoFar, int k) const;

    int totalContexts; // number of unique context paths stored (for stats)
};

#endif
