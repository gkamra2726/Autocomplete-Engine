#include "predictor.hpp"
#include "tokenizer.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

// UI Suggestions boundaries metadata tracker
struct ClickableBox {
    std::string word;
    float x;
    float y;
    float width;
    float height;
};

std::vector<ClickableBox> activeSuggestionBoxes;

// Helper function to filter out dataset specific technical noise/symbols
bool isValidWord(const std::string& str) {
    if (str.empty() || str == "<unk>" || str.find('@') != std::string::npos) return false;
    return true;
}

int main() {
    // Fast I/O operations optimization
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    // --- 1. Backend Predictive Initializations (Stream Loading Without Overriding Sort) ---
    Predictor predictor(4, 5); // Context size 4, max 5 high-quality structural suggestions
    
    std::string corpusPath = "data/wiki.train.tokens";
    std::ifstream file(corpusPath);
    
    if (!file.is_open()) {
        std::cerr << "Could not open large corpus file! Check path or filename.\n";
        return 1;
    }

    std::cout << "Loading dataset context trees... Please wait...\n";
    std::string line;
    size_t lineCount = 0;
    size_t maxLines = 150000; // Efficient limit context build
    
    while (std::getline(file, line) && lineCount < maxLines) {
        if (!line.empty()) {
            std::vector<std::string> rawTokens = tokenize(line);
            std::vector<std::string> cleanTokens;
            
            for (const auto& token : rawTokens) {
                if (isValidWord(token)) {
                    cleanTokens.push_back(token);
                }
            }
            if (!cleanTokens.empty()) {
                predictor.train(cleanTokens);
            }
        }
        lineCount++;
    }
    file.close();
    std::cout << "Dataset Loaded Successfully! Launching UI Console...\n";

    // --- 2. Micro-Frontend Premium Window Geometry ---
    sf::RenderWindow window(sf::VideoMode(1000, 600), "Core Engine v2.0 - Web Adaptive UI", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    // --- 3. Dynamic Typeface Asset Loading ---
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        if (!font.loadFromFile("arial.ttf")) {
            std::cerr << "Required interface fonts are missing!\n";
            return 1;
        }
    }

    // --- 4. Web-Standard Structural Branding Styles ---
    sf::Text heading("ENGINE CONSOLE", font, 14);
    heading.setFillColor(sf::Color(56, 189, 248)); // Tailwind Sky-400
    heading.setPosition(65, 50);
    heading.setStyle(sf::Text::Bold);

    sf::RectangleShape statusDot(sf::Vector2f(8.0f, 8.0f));
    statusDot.setFillColor(sf::Color(34, 197, 94)); // Live Green Node
    statusDot.setPosition(210, 55);

    sf::Text statusText("ONLINE", font, 11);
    statusText.setFillColor(sf::Color(34, 197, 94));
    statusText.setPosition(225, 52);
    statusText.setStyle(sf::Text::Bold);

    sf::Text subHeading("Type parameters. Use Left/Right arrows to move cursor. [TAB] or click options to complete.", font, 12);
    subHeading.setFillColor(sf::Color(148, 163, 184)); // Slate-400
    subHeading.setPosition(65, 80);

    // Modern Web Input Control Box Border Frame
    sf::RectangleShape inputContainer(sf::Vector2f(870.0f, 65.0f));
    inputContainer.setFillColor(sf::Color(30, 41, 59)); // Slate-800 Card Base
    inputContainer.setOutlineColor(sf::Color(71, 85, 105)); // Slate-600 Subtle Outline
    inputContainer.setOutlineThickness(1.0f);
    inputContainer.setPosition(65, 140);

    // Live Caret Input Text Field Display
    std::string currentInput = "";
    size_t cursorIndex = 0; // Navigation tracker index inside input string
    
    sf::Text inputText("", font, 18);
    inputText.setFillColor(sf::Color(241, 245, 249)); // Slate-100 High Contrast Text
    inputText.setPosition(85, 160);

    // Fixed Visual Shape Cursor to prevent character smudging/blurring
    sf::RectangleShape visualCursor(sf::Vector2f(2.0f, 22.0f));
    visualCursor.setFillColor(sf::Color(56, 189, 248)); // Matching Neon Cyan

    sf::Clock cursorClock;
    bool showCursor = true;

    sf::Text sugLabel("CONTEXT SUGGESTIONS", font, 11);
    sugLabel.setFillColor(sf::Color(148, 163, 184));
    sugLabel.setPosition(65, 260);
    sugLabel.setStyle(sf::Text::Bold);

    std::vector<std::string> currentSuggestions;

    // Lambda utility block to fetch valid contextual predictions preserving native Trie weight structure
    auto getCleanPredictions = [&](const std::string& inputStr) -> std::vector<std::string> {
        std::vector<std::string> rawSugs = predictor.predict(inputStr);
        std::vector<std::string> cleanSugs;
        
        for (const auto& s : rawSugs) {
            if (isValidWord(s)) {
                // Check for duplicate injection prevention in selection queue
                if (std::find(cleanSugs.begin(), cleanSugs.end(), s) == cleanSugs.end()) {
                    cleanSugs.push_back(s);
                }
            }
        }
        if (cleanSugs.size() > 5) {
            cleanSugs.resize(5); // Cap to premium top 5 items
        }
        return cleanSugs;
    };

    // --- 5. Application Event Dispatch Loop ---
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Click Handler Integration
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    float mX = static_cast<float>(event.mouseButton.x);
                    float mY = static_cast<float>(event.mouseButton.y);

                    for (const auto& box : activeSuggestionBoxes) {
                        if (mX >= box.x && mX <= box.x + box.width &&
                            mY >= box.y && mY <= box.y + box.height) {
                            
                            std::vector<std::string> currentTokens = tokenize(currentInput.substr(0, cursorIndex));
                            bool midWord = (cursorIndex > 0) && !std::isspace(static_cast<unsigned char>(currentInput[cursorIndex - 1]));

                            if (midWord && !currentTokens.empty()) {
                                size_t lastLen = currentTokens.back().length();
                                std::string leftPart = currentInput.substr(0, cursorIndex - lastLen);
                                std::string rightPart = currentInput.substr(cursorIndex);
                                currentInput = leftPart + box.word + " " + rightPart;
                                cursorIndex = leftPart.length() + box.word.length() + 1;
                            } else {
                                std::string leftPart = currentInput.substr(0, cursorIndex);
                                std::string rightPart = currentInput.substr(cursorIndex);
                                currentInput = leftPart + box.word + " " + rightPart;
                                cursorIndex = leftPart.length() + box.word.length() + 1;
                            }

                            currentSuggestions = getCleanPredictions(currentInput.substr(0, cursorIndex));
                            break;
                        }
                    }
                }
            }

            // Key Navigation and TAB Action Modules
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Tab) {
                    if (!currentSuggestions.empty()) {
                        std::vector<std::string> currentTokens = tokenize(currentInput.substr(0, cursorIndex));
                        bool midWord = (cursorIndex > 0) && !std::isspace(static_cast<unsigned char>(currentInput[cursorIndex - 1]));

                        if (midWord && !currentTokens.empty()) {
                            size_t lastLen = currentTokens.back().length();
                            std::string leftPart = currentInput.substr(0, cursorIndex - lastLen);
                            std::string rightPart = currentInput.substr(cursorIndex);
                            currentInput = leftPart + currentSuggestions[0] + " " + rightPart;
                            cursorIndex = leftPart.length() + currentSuggestions[0].length() + 1;
                        } else {
                            std::string leftPart = currentInput.substr(0, cursorIndex);
                            std::string rightPart = currentInput.substr(cursorIndex);
                            currentInput = leftPart + currentSuggestions[0] + " " + rightPart;
                            cursorIndex = leftPart.length() + currentSuggestions[0].length() + 1;
                        }
                        
                        currentSuggestions = getCleanPredictions(currentInput.substr(0, cursorIndex));
                    }
                }
                // Move Cursor Left
                else if (event.key.code == sf::Keyboard::Left) {
                    if (cursorIndex > 0) {
                        cursorIndex--;
                        currentSuggestions = getCleanPredictions(currentInput.substr(0, cursorIndex));
                    }
                }
                // Move Cursor Right
                else if (event.key.code == sf::Keyboard::Right) {
                    if (cursorIndex < currentInput.length()) {
                        cursorIndex++;
                        currentSuggestions = getCleanPredictions(currentInput.substr(0, cursorIndex));
                    }
                }
            }

            // Key Typings Processing Buffer Stream
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode != 9) { // Ignore TAB char
                    if (event.text.unicode == 8) { // Backspace processing
                        if (cursorIndex > 0) {
                            currentInput.erase(cursorIndex - 1, 1);
                            cursorIndex--;
                        }
                    }
                    else if (event.text.unicode == 13) { // Enter to reset console buffer
                        currentInput.clear();
                        cursorIndex = 0;
                    }
                    else if (event.text.unicode >= 32 && event.text.unicode <= 126) {
                        currentInput.insert(cursorIndex, 1, static_cast<char>(event.text.unicode));
                        cursorIndex++;
                    }
                }
                currentSuggestions = getCleanPredictions(currentInput.substr(0, cursorIndex));
            }
        }

        // Timer tracker loop for real-time cursor blinking (Every 450ms)
        if (cursorClock.getElapsedTime().asMilliseconds() >= 450) {
            showCursor = !showCursor;
            cursorClock.restart();
        }

        // Clean static base assignment
        inputText.setString(currentInput);

        // Dynamic Position Mapper for the Visual Overlay Cursor Shape
        sf::Text tempText(currentInput.substr(0, cursorIndex), font, 18);
        float cursorXOffset = tempText.findCharacterPos(cursorIndex).x;
        visualCursor.setPosition(85.0f + cursorXOffset, 163.0f);

        // --- 6. Hardware Accelerated View Layer Dispatching ---
        window.clear(sf::Color(2, 6, 23)); // Premium Tailwind Slate-950 Canvas Layout

        // Static Assets Draw Pipeline
        window.draw(heading);
        window.draw(statusDot);
        window.draw(statusText);
        window.draw(subHeading);
        window.draw(inputContainer);
        window.draw(inputText);
        window.draw(sugLabel);

        // Render Independent Blinking Cursor Rectangle
        if (showCursor) {
            window.draw(visualCursor);
        }

        // --- 7. Dynamic Flex-Style Capsules Engine ---
        activeSuggestionBoxes.clear();
        float boxX = 65.0f;
        float boxY = 295.0f;
        float padX = 22.0f;
        float padY = 12.0f;
        float gapBetween = 14.0f;

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        for (size_t i = 0; i < currentSuggestions.size(); ++i) {
            std::string currentWord = currentSuggestions[i];
            std::string displayString = (i == 0) ? currentWord + "  TAB" : currentWord;
            
            sf::Text textItem(displayString, font, 13);
            sf::FloatRect textDim = textItem.getLocalBounds();
            
            float cardWidth = textDim.width + (2 * padX);
            float cardHeight = textDim.height + (2 * padY);

            sf::RectangleShape visualCard(sf::Vector2f(cardWidth, cardHeight));
            
            bool isHovered = (mousePos.x >= boxX && mousePos.x <= boxX + cardWidth &&
                              mousePos.y >= boxY && mousePos.y <= boxY + cardHeight);

            if (i == 0) {
                visualCard.setFillColor(isHovered ? sf::Color(79, 70, 229) : sf::Color(99, 102, 241));
                textItem.setFillColor(sf::Color(255, 255, 255));
                textItem.setStyle(sf::Text::Bold);
            } else {
                visualCard.setFillColor(isHovered ? sf::Color(51, 65, 85) : sf::Color(30, 41, 59));
                visualCard.setOutlineColor(isHovered ? sf::Color(148, 163, 184, 150) : sf::Color(71, 85, 105, 100));
                visualCard.setOutlineThickness(1.0f);
                textItem.setFillColor(sf::Color(203, 213, 225));
            }

            visualCard.setPosition(boxX, boxY);
            textItem.setPosition(boxX + padX, boxY + padY - textDim.top - 1);

            ClickableBox currentBoxData = { currentWord, boxX, boxY, cardWidth, cardHeight };
            activeSuggestionBoxes.push_back(currentBoxData);

            window.draw(visualCard);
            window.draw(textItem);

            boxX += cardWidth + gapBetween;
        }

        window.display();
    }

    return 0;
}