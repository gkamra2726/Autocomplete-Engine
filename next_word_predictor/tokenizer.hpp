#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <string>
#include <vector>

// Reads entire file content into a single string
std::string readFile(const std::string& path);

// Splits text into lowercase word tokens.
// Rules:
//  - Lowercases everything
//  - Keeps apostrophes inside words (don't, it's) so contractions stay intact
//  - Treats all other punctuation as a separator
//  - Drops empty tokens
std::vector<std::string> tokenize(const std::string& text);

#endif
