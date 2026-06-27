#include "predictor.hpp"
#include "tokenizer.hpp"
#include <cctype>
#include <algorithm>
#include <vector>
#include <string>

// Levenshtein/Edit Distance calculator for Spell-Correction Layer
int calculateLevenshtein(const std::string& s1, const std::string& s2) {
    int m = s1.length(), n = s2.length();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

    for (int i = 0; i <= m; i++) dp[i][0] = i;
    for (int j = 0; j <= n; j++) dp[0][j] = j;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
            }
        }
    }
    return dp[m][n];
}

Predictor::Predictor(int maxContextSize, int k)
    : contextTrie(maxContextSize), topK(k) {}

void Predictor::train(const std::vector<std::string>& tokens) {
    for (const std::string& word : tokens) {
        wordTrie.insert(word);
    }
    contextTrie.buildFromCorpus(tokens);
}

std::vector<std::string> Predictor::predict(const std::string& input) {
    std::vector<std::string> suggestions;
    std::vector<std::string> tokens = tokenize(input);
    
    if (tokens.empty()) return suggestions;

    bool midWord = !input.empty() && !std::isspace(static_cast<unsigned char>(input.back()));

    if (midWord) {
        std::string partial = tokens.back(); 
        
        // Context Extraction Layer
        std::vector<std::string> contextWords(tokens.begin(), tokens.end() - 1);
        std::vector<std::pair<std::string, int>> contextPredictions = contextTrie.predictNext(contextWords, 100);

        // Candidate Selection Layer
        std::vector<std::string> candidates = wordTrie.autocomplete(partial, 20);
        
        if (candidates.empty()) {
            std::vector<std::string> allVocabulary = wordTrie.autocomplete("", 1000);
            std::vector<std::pair<std::string, int>> distanceMatches;

            for (const std::string& word : allVocabulary) {
                int dist = calculateLevenshtein(partial, word);
                if (dist <= 2) { 
                    distanceMatches.push_back({word, dist});
                }
            }
            
            std::sort(distanceMatches.begin(), distanceMatches.end(),
                [](const auto& a, const auto& b) { return a.second < b.second; });

            for (const auto& match : distanceMatches) {
                candidates.push_back(match.first);
            }
        }

        // Dual-Layer Scoring Logic
        std::vector<std::pair<std::string, int>> rankedCandidates;
        for (const std::string& candidate : candidates) {
            int score = 0;
            for (const auto& pred : contextPredictions) {
                if (pred.first == candidate) {
                    score = pred.second; 
                    break;
                }
            }
            rankedCandidates.push_back({candidate, score});
        }

        std::sort(rankedCandidates.begin(), rankedCandidates.end(), 
            [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                if (a.second != b.second) return a.second > b.second;
                return a.first < b.first;
            });

        for (int i = 0; i < (int)rankedCandidates.size() && i < topK; i++) {
            suggestions.push_back(rankedCandidates[i].first);
        }

        if (suggestions.empty()) {
            suggestions = wordTrie.autocomplete(partial, topK);
        }
    } 
    else {
        // --- FIX FOR SPACE MODE MULTI-SUGGESTIONS ---
        // 1. Core context predictions direct fetch karo
        std::vector<std::pair<std::string, int>> predictions = contextTrie.predictNext(tokens, topK);
        for (const auto& pair : predictions) {
            suggestions.push_back(pair.first);
        }

        // 2. Fallback Padding: Agar context solid elements return nahi kar paata (suggestions < topK),
        // toh generic vocabulary items use karke list ko completely fill karo taaki panel par options dikhein.
        if ((int)suggestions.size() < topK) {
            std::vector<std::string> backupWords = wordTrie.autocomplete("", 20);
            for (const std::string& bWord : backupWords) {
                if ((int)suggestions.size() >= topK) break;
                
                // Duplicate check takki same elements baar-baar display na hon
                if (std::find(suggestions.begin(), suggestions.end(), bWord) == suggestions.end()) {
                    suggestions.push_back(bWord);
                }
            }
        }
    }

    return suggestions;
}

int Predictor::vocabularySize() const {
    return wordTrie.totalWords;
}

int Predictor::contextCount() const {
    return contextTrie.totalContexts;
}
