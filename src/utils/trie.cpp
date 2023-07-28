#include "trie.hpp"

Trie::Trie() {
  _entryNode = new trieNode();
}

Trie::~Trie() {
  destroyTrie();
  delete _entryNode;
  _entryNode = NULL;
}

bool Trie::addWord(std::string word, int id) {
  trieNode * currentNode = _entryNode;
  bool isNewWord = false;
  for (size_t i = 0; i < word.length(); i++) {
    if (currentNode->children.count(word.at(i)) > 0) {
      currentNode = currentNode->children[word.at(i)];
    } else {
      isNewWord = true;
      trieNode * newNode = new trieNode();
      newNode->value = std::string(1, word.at(i));
      currentNode->children.insert({word.at(i), newNode});
      newNode = NULL;
      currentNode = currentNode->children[word.at(i)];
    }
  }
  currentNode->isEndingNode = true;
  currentNode->wordId = id;
  currentNode = NULL;
  return isNewWord;
}

std::vector<trieWord> Trie::getWordsFromInput(std::string word) {
  std::vector<trieWord> wordsList;
  trieNode * currentNode = _entryNode;
  std::string builtString("");
  for (size_t i = 0; i < word.length(); i++) {
    if (currentNode->children.count(word.at(i)) > 0) {
      builtString += std::string(currentNode->value);
      currentNode = currentNode->children[word.at(i)];
    }
    else {
      return wordsList;
    }
  }

  std::vector<stackNode> stack;
  stackNode startNode(currentNode);
  stack.push_back(startNode);
  currentNode = NULL;
  while (stack.size() > 0) {
    if (!stack.back().processed) {
      stack.back().currentString += std::string(stack.back().currentLoc->value);
      if (stack.back().currentLoc->isEndingNode) {
        trieWord resultOut;
        resultOut.word = builtString + stack.back().currentString;
        resultOut.id = stack.back().currentLoc->wordId;
        wordsList.push_back(resultOut);
      }
      stack.back().processed = true;
    } else {
      if (stack.back().remainingSteps.size() > 0) {
        char nextStep = stack.back().remainingSteps.back();
        std::string passedString = stack.back().currentString;
        stack.back().remainingSteps.pop_back();
        stackNode nextNode(stack.back().currentLoc->children[nextStep]);
        stack.push_back(nextNode);
        stack.back().currentString = passedString;
      } else {
        stack.back().currentLoc = NULL;
        stack.pop_back();
      }
    }
  }

  return wordsList;
}

void Trie::destroyTrie() {
  std::vector<stackNode> stack;
  stackNode startNode(_entryNode);
  stack.push_back(startNode);
  while (stack.size() > 0) {
    if (stack.back().remainingSteps.size() > 0) {
      char nextStep = stack.back().remainingSteps.back();
      stack.back().remainingSteps.pop_back();
      stackNode nextNode(stack.back().currentLoc->children[nextStep]);
      stack.push_back(nextNode);
    } else {
      delete stack.back().currentLoc;
      stack.back().currentLoc = NULL;
      stack.pop_back();
    }
  }
  _entryNode = new trieNode();
}
