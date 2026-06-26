#ifndef PREDICTOR_HPP
#define PREDICTOR_HPP

#include "trie.hpp"
#include "ngram_trie.hpp"
#include <string>
#include <vector>

class Predictor {
private:
    Trie wordTrie;
    NGramTrie contextTrie;
    int topK;

public:
    Predictor(int maxContextSize, int k);

    // Trains both tries from a token stream (call once at startup)
    void train(const std::vector<std::string>& tokens);

    // Given the full text typed so far, returns top-k suggestions.
    // Decides internally whether to autocomplete the current word
    // or predict the next word, based on whether `input` ends in a space.
    std::vector<std::string> predict(const std::string& input) const;

    // Stats for display
    int vocabularySize() const;
    int contextCount() const;
};

#endif
