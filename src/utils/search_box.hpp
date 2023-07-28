#pragma once
#ifndef Search_Box_hpp
#define Search_Box_hpp
#include "../../imgui/imgui.h"
#include "../../imgui/imgui-SFML.h"
#include <string>
#include <vector>

//a search box
struct textBoxState {
  bool isPopupOpen = false;
  int  activeIdx = -1;
  int  clickedIdx = -1;
  bool selectionChanged = false;
  ImVec2 popupPos;
  ImVec2 popupSize;
  bool isWindowFocused;
  bool isPopupFocused;
  std::vector<std::string> entries;
};

void SetInputFromActiveIndex( ImGuiInputTextCallbackData* data, int entryIndex );
int InputCallback( ImGuiInputTextCallbackData* data );

void drawTextBox(textBoxState& state, bool& isFocused, char * name, char * hint, char * textBuffer, int bufferSize, bool &submitted);

void drawPopup(textBoxState& state, bool& isFocused);

#endif
