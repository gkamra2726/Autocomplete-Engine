#include "predictor.hpp"
#include "tokenizer.hpp"
#include <cctype>

Predictor::Predictor(int maxContextSize, int k)
    : contextTrie(maxContextSize), topK(k) {}

void Predictor::train(const std::vector<std::string>& tokens) {
    for (const std::string& word : tokens) {
        wordTrie.insert(word);
    }
    contextTrie.buildFromCorpus(tokens);
}

std::vector<std::string> Predictor::predict(const std::string& input) const {
    std::vector<std::string> suggestions;

    if (input.empty()) return suggestions;

    bool midWord = !std::isspace(static_cast<unsigned char>(input.back()));

    // Tokenize what's typed so far (this naturally splits into completed words,
    // dropping a trailing partial word from `tokens` only if input ends in a space;
    // if midWord, the last token IS the partial word the user is typing).
    std::vector<std::string> tokens = tokenize(input);

    if (tokens.empty()) return suggestions;

    if (midWord) {
        // CASE 1: user is still typing a word -> autocomplete using Word Trie
        std::string partial = tokens.back();
        suggestions = wordTrie.autocomplete(partial, topK);
    } else {
        // CASE 2: user just finished a word (trailing space) -> predict NEXT word
        // using Context Trie, with the last 1-2 completed words as context.
        auto preds = contextTrie.predictNext(tokens, topK);
        for (auto& p : preds) suggestions.push_back(p.first);
    }

    return suggestions;
}

int Predictor::vocabularySize() const {
    return wordTrie.totalWords;
}

int Predictor::contextCount() const {
    return contextTrie.totalContexts;
}
