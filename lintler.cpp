#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <iomanip>

bool validateXML(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open XML file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int lineNumber = 0;
    std::vector<std::string> openTags;

    while (std::getline(file, line)) {
        lineNumber++;
        std::smatch match;
        std::regex tagRegex("<(/?[a-zA-Z0-9]+)([^>]*)>");

        std::string::const_iterator searchStart(line.cbegin());
        while (std::regex_search(searchStart, line.cend(), match, tagRegex)) {

            std::string tag = match[1];

            if (tag[0] == '/') {
                if (openTags.empty() || openTags.back() != tag.substr(1)) {
                    std::cerr << "Error: Mismatched closing tag: " << tag << ", line " << lineNumber << std::endl;
                    return false;
                }
                openTags.pop_back();
            } else {
                openTags.push_back(tag);
            }
            searchStart = match.suffix().first;
        }

        if (line.find('&') != std::string::npos && line.find("&amp;") == std::string::npos) {
            std::cerr << "Error: Invalid character '&' found without proper escaping in XML file: " << filename << ", line " << lineNumber << std::endl;
            return false;
        }
    }

    if (!openTags.empty()) {
        std::cerr << "Error: Unclosed tags in XML file: " << filename << std::endl;
        return false;
    }

    return true;
}

bool validateJSON(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open JSON file: " << filename << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonContent = buffer.str();

    int openBrackets = 0;
    int openSquareBrackets = 0;
    int lineNumber = 1;

    for (char c : jsonContent) {
        if (c == '{') openBrackets++;
        if (c == '}') openBrackets--;
        if (c == '[') openSquareBrackets++;
        if (c == ']') openSquareBrackets--;
        if (c == '\n') lineNumber++;

        if (openBrackets < 0 || openSquareBrackets < 0) {
            std::cerr << "Error: Unbalanced brackets in JSON file, line " << lineNumber << ": " << filename << std::endl;
            return false;
        }
    }

    if (openBrackets != 0 || openSquareBrackets != 0) {
        std::cerr << "Error: Unbalanced brackets in JSON file: " << filename << std::endl;
        return false;
    }

    return true;
}

bool validateCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open CSV file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int expectedColumnCount = -1;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        lineNumber++;
        std::stringstream lineStream(line);
        std::string cell;
        int columnCount = 0;

        while (std::getline(lineStream, cell, ',')) {
            columnCount++;
            for (char c : cell) {
                if (c < 32 && c != '\t' && c != '\r' && c != '\n') {
                    std::cerr << "Error: Invalid character in CSV file: " << filename << ", line " << lineNumber << std::endl;
                    return false;
                }
            }
        }

        if (expectedColumnCount == -1) {
            expectedColumnCount = columnCount;
        } else if (columnCount != expectedColumnCount) {
            std::cerr << "Error: Inconsistent column count in CSV file: " << filename << ", line " << lineNumber << std::endl;
            return false;
        }
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename1> [filename2] ..." << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        std::string filename = argv[i];
        std::string extension = filename.substr(filename.find_last_of(".") + 1);

        std::cout << "Validating: " << filename << std::endl;

        bool isValid = false;
        if (extension == "xml" || extension == "XML") {
            isValid = validateXML(filename);
        } else if (extension == "json" || extension == "JSON") {
            isValid = validateJSON(filename);
        } else if (extension == "csv" || extension == "CSV") {
            isValid = validateCSV(filename);
        } else {
            std::cerr << "Error: Unsupported file type: " << extension << std::endl;
            continue;
        }

        std::cout << "Validation: " << (isValid ? "Success" : "Failure") << std::endl;
        std::cout << std::setfill('-') << std::setw(30) << "-" << std::endl;
    }

    return 0;
}