#pragma once
#ifndef Console_hpp
#define Console_hpp
#include "../../imgui/imgui.h"
#include "../../imgui/imgui-SFML.h"
#include "../utils/search_box.hpp"
#include "../utils/trie.hpp"
#include "../utils/split.hpp"
#include "state.hpp"
#include "command.hpp"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <iostream>
#include <algorithm>

class Console {
public:
  Console(State * state);
  ~Console();

  void display();
  void runCommand(Split commandData);
  std::size_t findSpaceBefore(std::string wData, std::size_t before);
  void wrap(std::string wData);
  void write(std::string wData, std::string messageStart = "-> ");

private:
  std::vector<std::string> _output;
  State * _state;

  Command root;
  Trie _commandTrie;
  textBoxState _commandTrieBox;
  char _commandTextBuffer[512] = {0};

  bool scrollToBottom = false;
  bool setScrollOnChange = true;

  int scrollUp = 100;
};

#endif /* end of include guard:  */
