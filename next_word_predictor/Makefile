CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

CORE_SRCS = trie.cpp ngram_trie.cpp tokenizer.cpp predictor.cpp

all: app test

app: $(CORE_SRCS) main.cpp
	$(CXX) $(CXXFLAGS) $(CORE_SRCS) main.cpp -o predictor_app

test: $(CORE_SRCS) test_predictor.cpp
	$(CXX) $(CXXFLAGS) $(CORE_SRCS) test_predictor.cpp -o test_predictor

run: app
	./predictor_app

run-test: test
	./test_predictor

clean:
	rm -f predictor_app test_predictor

.PHONY: all run run-test clean
