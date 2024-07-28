#pragma once
#ifndef File_hpp
#define File_hpp
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

//file loading class
class File {
public:
  std::string getLine(int i);
  size_t size();
  void appendLine(std::string line);
  void loadFile(std::string fileLocation);
  void saveFile(std::string fileLocation);
  void clearFile();
private:
  std::vector<std::string> _lines;
};


#endif