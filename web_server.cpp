#include "predictor.hpp"
#include "tokenizer.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <csignal>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace {

bool isValidWord(const std::string& str) {
    return !str.empty() && str != "<unk>" && str.find('@') == std::string::npos;
}

std::string urlDecode(const std::string& value) {
    std::string decoded;
    decoded.reserve(value.size());

    for (size_t i = 0; i < value.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(value[i]);
        if (c == '+') {
            decoded.push_back(' ');
        } else if (c == '%' && i + 2 < value.size() && std::isxdigit(static_cast<unsigned char>(value[i + 1])) && std::isxdigit(static_cast<unsigned char>(value[i + 2]))) {
            std::string hex = value.substr(i + 1, 2);
            char* end = nullptr;
            long parsed = std::strtol(hex.c_str(), &end, 16);
            decoded.push_back(static_cast<char>(parsed));
            i += 2;
        } else {
            decoded.push_back(static_cast<char>(c));
        }
    }

    return decoded;
}

std::string jsonEscape(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size() + 8);

    for (char c : value) {
        switch (c) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    escaped += ' ';
                } else {
                    escaped.push_back(c);
                }
        }
    }

    return escaped;
}

std::string readHtml() {
    std::ifstream file("web/index.html");
    if (!file.is_open()) {
        return "<html><body><h1>Missing web/index.html</h1></body></html>";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string buildSuggestionsJson(const std::vector<std::string>& suggestions) {
    std::string json = "{\"suggestions\":[";
    for (size_t i = 0; i < suggestions.size(); ++i) {
        if (i > 0) json += ',';
        json += '"' + jsonEscape(suggestions[i]) + '"';
    }
    json += "]}";
    return json;
}

std::string getQueryValue(const std::string& requestTarget, const std::string& key) {
    const std::string needle = key + "=";
    size_t start = requestTarget.find(needle);
    if (start == std::string::npos) return {};
    start += needle.size();
    size_t end = requestTarget.find('&', start);
    std::string raw = requestTarget.substr(start, end == std::string::npos ? std::string::npos : end - start);
    return urlDecode(raw);
}

std::string buildHttpResponse(const std::string& status, const std::string& contentType, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << body;
    return response.str();
}

std::ifstream openCorpusStream() {
    const char* corpusPaths[] = {
        "data/wiki.train.tokens/wiki.train.tokens",
        "data/wiki.train.tokens",
        "data/sample.txt"
    };

    for (const char* path : corpusPaths) {
        std::ifstream file(path);
        if (file.is_open()) {
            std::cout << "Loading corpus from " << path << "\n";
            return file;
        }
    }

    throw std::runtime_error("Could not open a corpus file in data/");
}

void loadCorpus(Predictor& predictor) {
    std::ifstream file = openCorpusStream();

    std::string line;
    size_t lineCount = 0;
    const size_t maxLines = 150000;

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
        ++lineCount;
    }
}

} // namespace

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Predictor predictor(4, 5);
    try {
        std::cout << "Loading corpus for web UI...\n";
        loadCorpus(predictor);
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 1;
    }

    int serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        std::perror("socket");
        return 1;
    }

    int reuse = 1;
    ::setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(8080);

    if (::bind(serverFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::perror("bind");
        ::close(serverFd);
        return 1;
    }

    if (::listen(serverFd, 16) < 0) {
        std::perror("listen");
        ::close(serverFd);
        return 1;
    }

    std::signal(SIGPIPE, SIG_IGN);

    std::cout << "Web UI ready at http://127.0.0.1:8080\n";

    while (true) {
        sockaddr_in clientAddress{};
        socklen_t clientLength = sizeof(clientAddress);
        int clientFd = ::accept(serverFd, reinterpret_cast<sockaddr*>(&clientAddress), &clientLength);
        if (clientFd < 0) {
            continue;
        }

        std::string request;
        char buffer[4096];
        ssize_t bytesRead = 0;
        while ((bytesRead = ::recv(clientFd, buffer, sizeof(buffer), 0)) > 0) {
            request.append(buffer, buffer + bytesRead);
            if (request.find("\r\n\r\n") != std::string::npos) {
                break;
            }
        }

        std::string response;
        if (request.rfind("GET /api/suggest", 0) == 0) {
            size_t lineEnd = request.find('\r');
            std::string requestLine = request.substr(0, lineEnd);
            size_t pathStart = requestLine.find(' ') + 1;
            size_t pathEnd = requestLine.find(' ', pathStart);
            std::string requestTarget = requestLine.substr(pathStart, pathEnd - pathStart);
            std::string input = getQueryValue(requestTarget, "q");
            auto suggestions = predictor.predict(input);
            response = buildHttpResponse("200 OK", "application/json; charset=utf-8", buildSuggestionsJson(suggestions));
        } else if (request.rfind("GET /", 0) == 0) {
            response = buildHttpResponse("200 OK", "text/html; charset=utf-8", readHtml());
        } else {
            response = buildHttpResponse("404 Not Found", "text/plain; charset=utf-8", "Not Found");
        }

        ::send(clientFd, response.c_str(), response.size(), 0);
        ::close(clientFd);
    }

    ::close(serverFd);
    return 0;
}