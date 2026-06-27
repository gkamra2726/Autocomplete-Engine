# Core Engine v2.0 - Web Adaptive Predictive UI

A high-performance, real-time word autocomplete and next-word prediction interface built using C++ and SFML. The backend leverages an optimized N-Gram Trie language model trained on massive text corpora to instantly serve predictive textual context as you type.

---

## 🚀 Key Features

- **Optimized Stream Loading:** Engineered with memory-efficient line-by-line stream reading (`std::getline`), allowing the system to seamlessly ingest massive text files (such as the 539MB Wiki dataset) without RAM overflow or execution lag.
- **Native Contextual Ordering:** Preserves the native probabilistic weight and structure of the Trie graph, eliminating looping repetitions and providing grammatically sound next-word suggestions.
- **Micro-Frontend Premium UI:** Clean, dark-mode adaptive console layout styled after standard web components (Tailwind-inspired aesthetics).
- **Smooth Cursor Navigation:** Integrated full caret/cursor support with dedicated Left/Right arrow key navigation handling.
- **Frictionless Completion:** Quick autocompletion toggle using the `[TAB]` key or direct mouse clicks on the dynamic responsive visual capsule cards.

---

## 📦 Dataset Download & Setup

The predictive engine is optimized to utilize the comprehensive Wikipedia language modeling corpus. To set up the engine with the full-scale dataset:

1. Create a `data/` directory inside your project root folder.
2. Download the exact `wiki.train.tokens` dataset from Kaggle or the direct mirror link:
   - 🌐 Kaggle Link: https://www.kaggle.com/datasets/rohitgr/wikitext
3. Extract the downloaded dataset archive and place the largest file named `wiki.train.tokens` directly into your `data/` folder:

---

## 🛠️ Running the Project

This project is intended to be run using WSL (Windows Subsystem for Linux).

### Steps

1. Open your WSL terminal.
2. Navigate to the project directory.
3. Run the following commands: "make clean" and "make run-gui"

The `make clean` command removes previously generated build files, and `make run` builds and executes the project.

---

## 🕹️ Interface Controls

- **Keyboard Typing** — Input standard alpha-numeric parameters directly into the live viewport.
- **Left / Right Arrow Keys** — Intuitively traverse your cursor caret through characters.
- **TAB Key** — Instantly applies the primary high-priority recommendation card.
- **Left Mouse Click** — Select any contextual recommendation capsule directly using your mouse.
- **ENTER Key** — Completely clears the workspace buffer console stream to reset parameters.
