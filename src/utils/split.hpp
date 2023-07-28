#pragma once
#ifndef Split_hpp
#define Split_hpp
#include <string>
#include <vector>

class Split {
public:
  Split() {
    length=0;
  }
	Split(std::string myString, std::string delim) {
		size_t pos = myString.find(delim);
		while (pos != std::string::npos) {
			_strList.push_back(myString.substr(0, pos));
			myString.erase(0, pos+delim.length());
			pos = myString.find(delim);
		}
		_strList.push_back(myString);
		length = _strList.size();
	}

	std::string operator[](int id) {
		return _strList[id];
	}

	int length = 0;

private:
	std::vector<std::string> _strList;
};

#endif
