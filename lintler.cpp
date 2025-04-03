#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

bool validateXML(const string &filename) {
  ifstream file(filename);
  if (!file.is_open()) {
    cerr << "Error: Could not open XML file: " << filename << endl;
    return false;
  }

  string line;
  int lineNumber = 0;
  vector<string> openTags;
  regex tagRegex("<(/?[a-zA-Z0-9]+)([^>]*)>");

  while (getline(file, line)) {
    lineNumber++;
    smatch match;

    string::const_iterator searchStart(line.cbegin());
    while (regex_search(searchStart, line.cend(), match, tagRegex)) {
      string tag = match[1];

      if (tag[0] == '/') {
        if (openTags.empty() || openTags.back() != tag.substr(1)) {
          cerr << "Error: Mismatched closing tag: " << tag
                    << ", line: " << lineNumber << endl;
          return false;
        }
        openTags.pop_back();
      } else {
        openTags.push_back(tag);
      }
      searchStart = match.suffix().first;
    }

    for (size_t i = 0; i < line.length(); ++i) {
      if (line[i] == '&' && line.substr(i, 5) != "&amp;") {
        cerr << "Error: Invalid character '&' found without proper "
                     "escaping, line: "
                  << lineNumber << endl;
        return false;
      }

      bool insideTag = false;
      for (size_t i = 0; i < line.length(); ++i) {
        if (line[i] == '<') {
          if (insideTag) {
            cerr << "Error: Nested '<' found inside a tag, line: "
                      << lineNumber << endl;
            return false;
          }
          insideTag = true;
        } else if (line[i] == '>') {
          if (!insideTag) {
            cerr
                << "Error: Invalid character '>' found in text content, line: "
                << lineNumber << endl;
            return false;
          }
          insideTag = false;
        } else if (!insideTag && line[i] == '<') {
          cerr
              << "Error: Invalid character '<' found in text content, line: "
              << lineNumber << endl;
          return false;
        }
      }

      if (line[i] == '>' && line.substr(i, 4) != "&gt;") {
        if (i == 0 || !regex_search(line.substr(0, i + 1), tagRegex)) {
          cerr << "Error: Invalid character '>' found without proper "
                       "escaping, line: "
                    << lineNumber << endl;
          return false;
        }
      }
    }
  }

  if (!openTags.empty()) {
    cerr << "Error: Unclosed tags in XML file: " << filename << endl;
    return false;
  }

  return true;
}

bool validateJSON(const string &filename) {
  ifstream file(filename);
  if (!file.is_open()) {
    cerr << "Error: Could not open JSON file: " << filename << endl;
    return false;
  }

  stringstream buffer;
  buffer << file.rdbuf();
  string jsonContent = buffer.str();

  int openBrackets = 0;
  int openSquareBrackets = 0;
  int lineNumber = 1;

  for (char c : jsonContent) {
    if (c == '{')
      openBrackets++;
    if (c == '}')
      openBrackets--;
    if (c == '[')
      openSquareBrackets++;
    if (c == ']')
      openSquareBrackets--;
    if (c == '\n')
      lineNumber++;

    if (openBrackets < 0 || openSquareBrackets < 0) {
      cerr << "Error: Unbalanced brackets in JSON file, line "
                << lineNumber << ": " << filename << endl;
      return false;
    }
  }

  if (openBrackets != 0 || openSquareBrackets != 0) {
    cerr << "Error: Unbalanced brackets in JSON file: " << filename
              << endl;
    return false;
  }

  return true;
}

bool validateCSV(const string &filename) {
  ifstream file(filename);
  if (!file.is_open()) {
    cerr << "Error: Could not open CSV file: " << filename << endl;
    return false;
  }

  string line;
  int expectedColumnCount = -1;
  int lineNumber = 0;

  while (getline(file, line)) {
    lineNumber++;
    stringstream lineStream(line);
    string cell;
    int columnCount = 0;

    while (getline(lineStream, cell, ',')) {
      columnCount++;
      for (char c : cell) {
        if (c < 32 && c != '\t' && c != '\r' && c != '\n') {
          cerr << "Error: Invalid character in CSV file: " << filename
                    << ", line " << lineNumber << endl;
          return false;
        }
      }
    }

    if (expectedColumnCount == -1) {
      expectedColumnCount = columnCount;
    } else if (columnCount != expectedColumnCount) {
      cerr << "Error: Inconsistent column count in CSV file: " << filename
                << ", line " << lineNumber << endl;
      return false;
    }
  }

  return true;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <filename1> [filename2] ..."
              << endl;
    return 1;
  }

  for (int i = 1; i < argc; ++i) {
    string filename = argv[i];
    string extension = filename.substr(filename.find_last_of(".") + 1);

    cout << "Validating: " << filename << endl;

    bool isValid = false;
    if (extension == "xml" || extension == "XML") {
      isValid = validateXML(filename);
    } else if (extension == "json" || extension == "JSON") {
      isValid = validateJSON(filename);
    } else if (extension == "csv" || extension == "CSV") {
      isValid = validateCSV(filename);
    } else {
      cerr << "Error: Unsupported file type: " << extension << endl;
      continue;
    }

    cout << "Validation: " << (isValid ? "Success" : "Failure")
              << endl;
    cout << setfill('-') << setw(30) << "-" << endl;
  }

  return 0;
}