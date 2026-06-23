#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <emscripten/emscripten.h>

using namespace std;

struct TrieNode
{
    unordered_map<char, TrieNode *> children;
    bool isEndOfWord;
    long long frequency;

    TrieNode()
    {
        isEndOfWord = false;
        frequency = 0;
    }
};

class IntelligentTextEngine
{
private:
    TrieNode *root;
    vector<pair<string, long long>> allWordsCorpus;

    int getEditDistance(const string &s1, const string &s2)
    {
        int m = s1.length(), n = s2.length();
        if (abs(m - n) > 2)
            return 99;

        vector<vector<int>> dp(m + 1, vector<int>(n + 1));
        for (int i = 0; i <= m; i++)
            dp[i][0] = i;
        for (int j = 0; j <= n; j++)
            dp[0][j] = j;

        for (int i = 1; i <= m; i++)
        {
            for (int j = 1; j <= n; j++)
            {
                if (s1[i - 1] == s2[j - 1])
                {
                    dp[i][j] = dp[i - 1][j - 1];
                }
                else
                {
                    dp[i][j] = 1 + min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
                }
            }
        }
        return dp[m][n];
    }

    void collectWords(TrieNode *curr, string currentPrefix, vector<pair<string, long long>> &results)
    {
        if (!curr)
            return;
        if (curr->isEndOfWord)
        {
            results.push_back({currentPrefix, curr->frequency});
        }
        for (auto const &[ch, childNode] : curr->children)
        {
            collectWords(childNode, currentPrefix + ch, results);
        }
    }

public:
    IntelligentTextEngine()
    {
        root = new TrieNode();
    }

    void insert(string word, long long freq)
    {
        TrieNode *curr = root;
        for (char ch : word)
        {
            if (curr->children.find(ch) == curr->children.end())
            {
                curr->children[ch] = new TrieNode();
            }
            curr = curr->children[ch];
        }
        curr->isEndOfWord = true;
        curr->frequency = max(curr->frequency, freq);

        // Check if already exists in baseline corpus to prevent redundant tracking pointers
        for (auto &entry : allWordsCorpus)
        {
            if (entry.first == word)
            {
                entry.second = max(entry.second, freq);
                return;
            }
        }
        allWordsCorpus.push_back({word, freq});
    }

    // Dynamic self-learning engine update method
    void reinforceFrequency(string word)
    {
        TrieNode *curr = root;
        for (char ch : word)
        {
            if (curr->children.find(ch) == curr->children.end())
                return; // Word mismatch safety fallback
            curr = curr->children[ch];
        }
        if (curr->isEndOfWord)
        {
            // Boost frequency scaling metrics by a significant coefficient step
            curr->frequency += 10000000;

            // Sync cross-reference values inside search loop indices
            for (auto &entry : allWordsCorpus)
            {
                if (entry.first == word)
                {
                    entry.second = curr->frequency;
                    break;
                }
            }
        }
    }

    string getAutoComplete(string prefix)
    {
        TrieNode *curr = root;
        bool structuralMismatch = false;

        for (char ch : prefix)
        {
            if (curr->children.find(ch) == curr->children.end())
            {
                structuralMismatch = true;
                break;
            }
            curr = curr->children[ch];
        }

        vector<pair<string, long long>> matchedResults;

        if (!structuralMismatch)
        {
            collectWords(curr, prefix, matchedResults);
            sort(matchedResults.begin(), matchedResults.end(), [](const pair<string, long long> &a, const pair<string, long long> &b)
                 { return a.second > b.second; });
        }
        else
        {
            vector<pair<string, pair<int, long long>>> autocorrectCandidates;
            for (const auto &entry : allWordsCorpus)
            {
                int distance = getEditDistance(prefix, entry.first);
                if (distance <= 2)
                {
                    autocorrectCandidates.push_back({entry.first, {distance, entry.second}});
                }
            }

            sort(autocorrectCandidates.begin(), autocorrectCandidates.end(), [](const auto &a, const auto &b)
                 {
                if (a.second.first != b.second.first) {
                    return a.second.first < b.second.first;
                }
                return a.second.second > b.second.second; });

            for (const auto &candidate : autocorrectCandidates)
            {
                matchedResults.push_back({candidate.first, candidate.second.second});
            }
        }

        string serializedOutput = "";
        int limit = min(5, (int)matchedResults.size());
        for (int i = 0; i < limit; i++)
        {
            serializedOutput += matchedResults[i].first;
            if (i < limit - 1)
                serializedOutput += "|";
        }
        return serializedOutput;
    }
};

IntelligentTextEngine engine;
string outputCache;

extern "C"
{

    EMSCRIPTEN_KEEPALIVE
    void init_dictionary()
    {
        ifstream file("dictionary.txt");
        if (!file.is_open())
        {
            engine.insert("computer", 950000);
            engine.insert("compiler", 150000);
            engine.insert("cache", 250000);
            return;
        }
        string word;
        long long freq;
        while (file >> word >> freq)
        {
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            engine.insert(word, freq);
        }
        file.close();
    }

    EMSCRIPTEN_KEEPALIVE
    const char *query_suggestions(const char *prefix)
    {
        string pStr = string(prefix);
        pStr.erase(0, pStr.find_first_not_of(" \t\r\n"));
        pStr.erase(pStr.find_last_not_of(" \t\r\n") + 1);
        outputCache = engine.getAutoComplete(pStr);
        return outputCache.c_str();
    }

    // Export link layer to register clicks from JavaScript runtime context
    EMSCRIPTEN_KEEPALIVE
    void increment_word_frequency(const char *word)
    {
        engine.reinforceFrequency(string(word));
    }
}