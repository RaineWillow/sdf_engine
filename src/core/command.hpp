#pragma once
#ifndef Command_hpp
#define Command_hpp
#include "state.hpp"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <iostream>
#include <algorithm>
#include <utility>
#include "../../imgui/imgui.h"
#include "../../imgui/imgui-SFML.h"
#include "../utils/search_box.hpp"
#include "../utils/trie.hpp"
#include "../utils/split.hpp"

class Console;

class Command {
public:
  void addName(std::string name, std::vector<std::string> names);
  bool hasParam(std::string name);
  void addParam(std::string name, std::function<void(State*, Split, Console*)> impl);
  void addInvocation(std::function<void(State*, Split, Console*)> impl);
  Command* operator[](std::string key);
  void operator()(State* state, Split args, Console * console);
private:
  std::vector<std::string> _names;
  std::map<std::string, Command> params;
  std::function<void(State*, Split, Console*)> invoke;
};
#endif
