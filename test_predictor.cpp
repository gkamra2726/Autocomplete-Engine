#include "predictor.hpp"
#include "tokenizer.hpp"
#include <iostream>

void show(Predictor& p, const std::string& input) {
    auto suggestions = p.predict(input);
    std::cout << "Input: \"" << input << "\"\n  -> [";
    for (size_t i = 0; i < suggestions.size(); i++) {
        std::cout << suggestions[i];
        if (i + 1 < suggestions.size()) std::cout << ", ";
    }
    std::cout << "]\n\n";
}

int main() {
    std::string rawText = readFile("data/sample.txt");
    std::vector<std::string> tokens = tokenize(rawText);

    Predictor predictor(2, 5);
    predictor.train(tokens);

    std::cout << "Vocabulary: " << predictor.vocabularySize()
              << " | Contexts: " << predictor.contextCount() << "\n\n";

    // Simulate typing "i like to e" character by character (key moments only)
    show(predictor, "i");          // mid-word, autocomplete "i" itself (trivial)
    show(predictor, "i ");         // next-word prediction after "i"
    show(predictor, "i like ");    // next-word after bigram "like"
    show(predictor, "i like to "); // next-word after trigram "like to"
    show(predictor, "i like to e"); // mid-word autocomplete for "e..." given trigram context ignored (word trie is context-blind)
    show(predictor, "i like to eat "); // next word after "eat"
    show(predictor, "the weather "); // bigram context
    show(predictor, "the weather is "); // trigram context
    show(predictor, "she ");
    show(predictor, "she likes to ");
    show(predictor, "zzzz "); // unseen word, should gracefully return nothing

    return 0;
}
