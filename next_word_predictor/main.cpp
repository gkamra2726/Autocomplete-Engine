#include "predictor.hpp"
#include "tokenizer.hpp"
#include <iostream>
#include <string>

// Clears the current line in terminal so we can redraw it (simple live-typing effect)
void clearLine() {
    std::cout << "\r\033[K";
}

void printSuggestions(const std::vector<std::string>& suggestions) {
    std::cout << " | suggestions: ";
    if (suggestions.empty()) {
        std::cout << "(none)";
    } else {
        for (size_t i = 0; i < suggestions.size(); i++) {
            std::cout << "[" << suggestions[i] << "]";
            if (i + 1 < suggestions.size()) std::cout << " ";
        }
    }
}

int main() {
    std::cout << "=== Next-Word Prediction & Autocomplete (Trie-based) ===\n";

    // --- Load and train ---
    std::string corpusPath = "data/sample.txt";
    std::string rawText = readFile(corpusPath);

    if (rawText.empty()) {
        std::cerr << "Could not read corpus file at " << corpusPath << "\n";
        return 1;
    }

    std::vector<std::string> tokens = tokenize(rawText);

    int maxContext = 2; // up to trigrams
    int topK = 5;
    Predictor predictor(maxContext, topK);
    predictor.train(tokens);

    std::cout << "Loaded corpus: " << tokens.size() << " tokens, "
              << predictor.vocabularySize() << " unique words, "
              << predictor.contextCount() << " context nodes.\n\n";

    std::cout << "Type a sentence. Suggestions update live after every character.\n";
    std::cout << "Type 'EXIT' on its own to quit.\n\n";

    std::string buffer;
    char ch;

    // Character-by-character input loop to simulate live typing.
    // (std::cin.get reads one char at a time, including spaces and newline.)
    while (std::cin.get(ch)) {
        if (ch == '\n') {
            // Check for exit command
            if (buffer == "EXIT") break;

            // On Enter: show final suggestions for the completed line, then reset
            std::cout << "\n";
            buffer.clear();
            std::cout << "> ";
            continue;
        }

        buffer += ch;

        auto suggestions = predictor.predict(buffer);

        clearLine();
        std::cout << "> " << buffer;
        printSuggestions(suggestions);
        std::cout.flush();
    }

    std::cout << "\nGoodbye!\n";
    return 0;
}
