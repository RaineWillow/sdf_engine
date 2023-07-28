#pragma once
#ifndef Trie_hpp
#define Trie_hpp
#include <string>
#include <vector>
#include <map>

//trie data structure
struct trieWord {
  std::string word;
  int id;
};

struct trieNode {
  std::map<char, trieNode*> children;
  std::string value;
  bool isEndingNode = false;
  int wordId = 0;
  trieNode() {
    this->value = "";
  }
};

struct stackNode {
  bool processed = false;
  std::vector<char> remainingSteps;
  std::string currentString;
  trieNode * currentLoc;
  stackNode(trieNode * location) {
    currentLoc = location;
    if (!location->children.empty()) {
      for (const auto &keys : location->children) {
        remainingSteps.push_back(keys.first);
      }
    }
  }
};

class Trie {
public:
  Trie();
  ~Trie();

  bool addWord(std::string word, int id);
  std::vector<trieWord> getWordsFromInput(std::string word);

  void destroyTrie();
private:
  trieNode * _entryNode;
};

#endif
