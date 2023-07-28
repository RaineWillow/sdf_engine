#include "console.hpp"
#include "command.hpp"

void Command::addName(std::string name, std::vector<std::string> names) {
  _names = names;
  _names.push_back(name);
}
bool Command::hasParam(std::string name) {
  return params.count(name)>0;
}
void Command::addParam(std::string name, std::function<void(State*, Split, Console*)> impl) {
  Command param;
  param.addInvocation(impl);
  param.addName(name, _names);
  params[name] = param;
}
void Command::addInvocation(std::function<void(State*, Split, Console*)> impl) {
  invoke = impl;
}
Command* Command::operator[](std::string key) {
  return &params[key];
}
void Command::operator()(State* state, Split args, Console * console) {
  invoke(state, args, console);
}
