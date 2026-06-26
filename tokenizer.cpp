#include "tokenizer.hpp"
#include <fstream>
#include <sstream>
#include <cctype>

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current;

    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        char lower = std::tolower(static_cast<unsigned char>(c));

        if (lower >= 'a' && lower <= 'z') {
            current += lower;
        } else if (c == '\'' && !current.empty()) {
            // keep apostrophes inside a word (don't, it's) but not at the start
            current += '\'';
        } else {
            // any other character (space, punctuation, digit, newline) ends the current word
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        }
    }

    if (!current.empty()) tokens.push_back(current);

    return tokens;
}
