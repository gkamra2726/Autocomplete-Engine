# Next-Word Prediction & Autocomplete (Trie-based DSA Project)

A C++ tool that predicts the next word and autocompletes the current word as
you type, using two trie-based data structures built from scratch.

## How it works

This project deliberately uses **two tries for two different jobs**, because
a plain trie only solves "complete the word I'm currently typing" — it has
no concept of word order or context. Predicting the *next* word requires
remembering sequences, which is a different structure.

### 1. Word Trie (`trie.hpp` / `trie.cpp`)
A classic 26-ary trie (`children[26]` per node, lowercase only). Stores every
word in the vocabulary with a frequency count. Used for **autocomplete**:
given a partial word like `"pro"`, it DFS-walks the subtree under that prefix
and returns the most frequent matches.

### 2. N-gram Context Trie (`ngram_trie.hpp` / `ngram_trie.cpp`)
A second trie where each **path of nodes represents a sequence of context
words** (e.g. `"i" -> "like" -> "to"`), and each node stores a frequency map
of `{next_word: count}` — i.e. "what word followed this exact context in the
training text, and how often."

This is built from the corpus by sliding a window across every position and
inserting context lengths from 1 up to `maxContext` (we use 2, giving
unigram/bigram/trigram contexts all in one trie). Storing all orders together
is what makes **backoff** possible:

> To predict the next word, try the longest context first (trigram). If that
> exact sequence was never seen, shrink to bigram. If still nothing, fall
> back to unigram. If even that's empty, no prediction is made — the system
> never guesses blindly.

### 3. Predictor (`predictor.hpp` / `predictor.cpp`)
Combines both tries and decides which one to use:
- If the user is **mid-word** (no trailing space) → Word Trie autocomplete
- If the user **just finished a word** (trailing space) → Context Trie next-word prediction, using the last 1-2 completed words as context

### 4. Tokenizer (`tokenizer.hpp` / `tokenizer.cpp`)
Lowercases text, keeps apostrophes inside words (`don't`, `it's`), and treats
everything else as a word separator.

## Build & run

```bash
make            # builds both predictor_app and test_predictor
make run        # interactive live-typing demo
make run-test   # non-interactive correctness demo (good for screenshots/report)
make clean
```

`predictor_app` is a character-by-character live demo: type a sentence and
watch suggestions update after every keystroke, switching automatically
between autocomplete and next-word prediction. Type `EXIT` to quit.

`test_predictor` runs a fixed set of inputs through the predictor and prints
results — useful for demonstrating correctness without needing a live typing
demo (e.g. in a report or recorded video).

## Project structure

```
next_word_predictor/
├── trie.hpp/.cpp           Word Trie (autocomplete)
├── ngram_trie.hpp/.cpp     Context Trie (next-word prediction + backoff)
├── tokenizer.hpp/.cpp      Text cleaning
├── predictor.hpp/.cpp      Combines both, routes input to the right trie
├── main.cpp                Interactive live-typing CLI
├── test_predictor.cpp      Non-interactive test harness
├── Makefile
└── data/
    └── sample.txt           Built-in corpus (~30 short paragraphs, repeated patterns)
```

## Swapping in a bigger dataset (e.g. a Gutenberg book)

1. Download any plain-text book from [Project Gutenberg](https://www.gutenberg.org)
   (e.g. *Alice in Wonderland*, plain `.txt` format).
2. Save it as `data/gutenberg_book.txt`.
3. In `main.cpp` (and/or `test_predictor.cpp`), change:
   ```cpp
   std::string corpusPath = "data/sample.txt";
   ```
   to:
   ```cpp
   std::string corpusPath = "data/gutenberg_book.txt";
   ```
4. Rebuild with `make clean && make all`.

Gutenberg books include a license header/footer — for a cleaner corpus, trim
the first/last ~30-50 lines (the boilerplate) before using it, though the
tokenizer will still work fine even if you don't.

## Complexity notes (for your report)

- **Word Trie insert**: O(L) per word, where L = word length
- **Word Trie autocomplete**: O(P + N) where P = prefix length (walk to node),
  N = number of nodes in the matching subtree (DFS collection), plus
  O(M log M) for sorting M matches by frequency
- **Context Trie insert**: O(maxContext) per token position, since we insert
  every context length from 1 to maxContext at each position → overall
  O(n · maxContext) to build from n tokens
- **Context Trie predict**: O(maxContext) for the longest lookup, with
  backoff adding at most maxContext additional lookups in the worst case →
  O(maxContext²) worst case, but maxContext is a small constant (2 here), so
  effectively O(1) per prediction

## Known limitations (good to mention in a viva/demo)

- Word Trie autocomplete is **context-blind** — typing `"i like to e"` will
  autocomplete based on letters alone (`every`, `eat`, `early`...), not
  weighted by the fact that "eat" is contextually far more likely after
  "like to". A stronger version could re-rank Word Trie completions using
  Context Trie frequencies — a natural "next step" to mention.
- Lowercase a-z only — no digits, no Unicode/accented characters.
- No smoothing beyond simple backoff (no Laplace/Kneser-Ney) — appropriate
  for a DSA-focused project, but worth naming as a simplification if asked.
