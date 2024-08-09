#include "console.hpp"
Console::Console(State * state) {
  _state = state;
  _commandTrieBox.popupSize = ImVec2(500, 50);

  //cat command-----------------------------------------------------------------
  state->addCommand("cat");
  root.addParam("cat", [](State* state, Split args, Console * console){
    console->write("Error! Command 'cat' requires at least one parameter!", "-! ");
  });
  Command* cat = root["cat"];
  cat->addParam("windowSize", [](State* state, Split args, Console * console) {
    console->write("Size of the window:\nWidth: " + std::to_string(state->windowWidth) + "\nHeight: " + std::to_string(state->windowHeight));
  });
  cat->addParam("help", [](State* state, Split args, Console * console) {
    std::string output = "cat [parameter]\n";
    output += "Parameters:\n";
    output += "'windowSize'";
    console->write(output);
  });
  
  //help command----------------------------------------------------------------
  state->addCommand("help");
  root.addParam("help", [](State* state, Split args, Console * console) {
    std::string output = "Commands list:\n";
    for (auto i : state->commandHelps) {
      output += i + "\n";
    }
    output += "Input commands by typing [command] [param1] [param2] [...] |[arg1],[arg2],[arg3],[...]";
    console->write(output);
  });
}

Console::~Console() {
  _state=NULL;
}


void Console::display() {
  ImGui::SetNextWindowPos(ImVec2(-1, 31));
  ImGui::SetNextWindowSize(ImVec2(515, 500));
  bool active = true;
  ImGui::Begin("Console", &active, ImGuiWindowFlags_NoResize);
  ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoTitleBar              |
      ImGuiWindowFlags_NoResize                |
      ImGuiWindowFlags_NoMove                  |
      ImGuiWindowFlags_AlwaysVerticalScrollbar |
      ImGuiWindowFlags_NoFocusOnAppearing      |
      ImGuiWindowFlags_NoSavedSettings;
  ImGui::BeginChild("data display", ImVec2(500, 300), true, flags);



  int displayStart = std::max(0, (int)_output.size()-scrollUp);
  int displayEnd =  std::min((int)_output.size(), displayStart+100);

  for (int i = displayStart; i < displayEnd; i++) {
    std::string fancyText = _output[i].substr(0, 3);
    std::string data = _output[i].substr(3);
    if (fancyText=="-$ ") {
      ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), "%s", fancyText.c_str());
    } else if (fancyText=="-> ") {
      ImGui::TextColored(ImVec4(0.8, 0.8, 0.8, 1.0), "%s", fancyText.c_str());
    } else if (fancyText=="-! ") {
      ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "%s", fancyText.c_str());
    } else {
      ImGui::TextColored(ImVec4(0.4, 0.4, 0.4, 1.0), "%s", fancyText.c_str());
    }
    ImGui::SameLine();
    ImGui::Text("%s", data.c_str());
  }

  int scrollPosY = ImGui::GetScrollY();

  if (scrollPosY <= 60 && displayStart != 0) {
    scrollUp += 10;
    ImGui::SetScrollY(scrollPosY + ImGui::GetTextLineHeightWithSpacing()*10);
  }

  if (scrollPosY >= 1350 && displayEnd != (int)_output.size()) {
    scrollUp -= 10;
    ImGui::SetScrollY(scrollPosY - ImGui::GetTextLineHeightWithSpacing()*10);
  }

  if (scrollToBottom) {
    ImGui::SetScrollHereY(1.0);
    scrollUp = 100;
    scrollToBottom = false;
  }
  ImGui::EndChild();

  if (ImGui::Button(std::string("Scroll on Change: " + std::string((setScrollOnChange) ? " enabled" : "disabled")).c_str())) {
    setScrollOnChange = !setScrollOnChange;
  }

  ImGui::SameLine();
  if (ImGui::Button("Jump to Bottom")) {
    scrollToBottom = true;
  }

  bool popupFocused = false;
  bool textBoxFocused = false;
  _commandTrieBox.isPopupOpen = true;
  char label[] = "##consoleTextBox";
  char hint[] = "-$";
  bool submitted = false;
  drawTextBox(_commandTrieBox, textBoxFocused, label, hint, _commandTextBuffer, 512, submitted);
  if (submitted) {
    this->write(std::string(_commandTextBuffer), "-$ ");
    Split commandData(std::string(_commandTextBuffer), " ");
    memset(_commandTextBuffer, '\0', sizeof(_commandTextBuffer));

    if (!root.hasParam(commandData[0])) {
      this->write("Error! Command '" + commandData[0] + "' does not exist. Try 'help' for a general command list.", "-! ");
    } else {
      runCommand(commandData);
    }
  }
  drawPopup(_commandTrieBox, popupFocused);
  ImGui::End();
}

void Console::runCommand(Split commandData) {
  Command* currentCommand = root[commandData[0]];

  Split args;

  for (int i = 1; i < commandData.length; i++) {
    if (commandData[i].substr(0, 1)=="|") {
      args = Split(commandData[i].substr(1), ",");
      break;
    }

    if (!currentCommand->hasParam(commandData[i])) {
      this->write("Error! Command '" + commandData[0] + "' does not have parameter '" + commandData[i] + "'.", "-! ");
      return;
    }

    currentCommand = (*currentCommand)[commandData[i]];
  }

  (*currentCommand)(_state, args, this);
}

std::size_t Console::findSpaceBefore(std::string wData, std::size_t before) {
  std::size_t foundSpace = wData.find(" ");
  if (foundSpace==std::string::npos) {
    return before;
  }
  if (foundSpace >= before) {
    return before;
  }
  std::size_t lastFound = foundSpace;
  foundSpace = wData.find(" ", lastFound+1);
  while (foundSpace!=std::string::npos) {
    //std::cout << "buggin out " << foundSpace << " before: " << before << std::endl;
    if (foundSpace == before) {
      return before;
    } else if (foundSpace > before) {
      //std::cout << "returned" << std::endl;
      break;
    }
    lastFound = foundSpace;
    foundSpace = wData.find(" ", lastFound+1);
  }
  if (lastFound > 20) {
    return lastFound;
  } else {
    return before;
  }
}

void Console::wrap(std::string wData) {
  std::string currentString = wData;
  if (currentString.length() > 67) {
    size_t spacePoint = findSpaceBefore(currentString.substr(3), 64)+3;
    std::string currentSlice = currentString.substr(0, spacePoint);
    _output.push_back(currentSlice);
    currentString = " * " + currentString.substr(std::min(spacePoint+1, currentString.length()-1), currentString.length()-spacePoint+1);
    while (currentString.length() > 67) {
      spacePoint = findSpaceBefore(currentString.substr(3), 64)+3;
      currentSlice = currentString.substr(0, spacePoint);
      _output.push_back(currentSlice);
      currentString = " * " + currentString.substr(std::min(spacePoint+1, currentString.length()-1), currentString.length()-spacePoint+1);
    }

    if (currentString.length() > 0) {
      _output.push_back(currentString);
    }
  } else {
    _output.push_back(currentString);
  }
}

void Console::write(std::string wData, std::string messageStart) {
  std::string currentString = messageStart + wData;
  std::cout << currentString << std::endl;
  std::size_t newLine = currentString.find("\n");
  if (newLine!=std::string::npos) {
    std::string currentSlice = currentString.substr(0, newLine);
    wrap(currentSlice);
    currentString = " * " + currentString.substr(std::min(newLine+1, currentString.length()-1), currentString.length()-newLine+1);
    newLine = currentString.find("\n");
    while (newLine!=std::string::npos) {
      currentSlice = currentString.substr(0, newLine);
      wrap(currentSlice);
      currentString = " * " + currentString.substr(std::min(newLine+1, currentString.length()-1), newLine+1-currentString.length());
      newLine = currentString.find("\n");
    }

    if (currentString.length() > 0) {
      wrap(currentString);
    }
  } else {
    wrap(currentString);
  }
  if (setScrollOnChange) {
      scrollToBottom = true;
  }
}
