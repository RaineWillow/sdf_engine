#include "file.hpp"

std::string File::getLine(int i) {
  return _lines[i];
}

size_t File::size() {
  return _lines.size();
}

void File::appendLine(std::string line) {
  _lines.push_back(line);
}

void File::loadFile(std::string fileLocation) {
  std::ifstream inputStream;
  inputStream.open(fileLocation);
  if (inputStream.fail()) {
    std::cout << "failed to read file: " + fileLocation << std::endl;
  }
  std::string fLine;
  _lines.clear();

  while (getline(inputStream, fLine)) {
    _lines.push_back(fLine);
  }
  inputStream.close();
}

void File::saveFile(std::string fileLocation) {
  std::ofstream outputStream;
  outputStream.open(fileLocation, std::ios::out | std::ios::trunc);
  if (outputStream.fail()) {
    std::cout << "failed to save file:" + fileLocation << std::endl;
  }
  for (size_t i = 0; i < _lines.size(); i++) {
    outputStream << _lines[i] << "\n";
  }
  outputStream.close();
}

void File::clearFile() {
  _lines.clear();
}