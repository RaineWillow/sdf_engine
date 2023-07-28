#include "search_box.hpp"
//-----------------------------------------------------------
void SetInputFromActiveIndex( ImGuiInputTextCallbackData* data, int entryIndex )
{
    textBoxState& state = *reinterpret_cast<textBoxState*>( data->UserData );

    const char*  entry  = state.entries[entryIndex].c_str();
    const size_t length = strlen( entry );

    memmove( data->Buf, entry, length + 1 );

    data->BufTextLen = (int)length;
    data->BufDirty   = true;
    data->CursorPos  = (int)length;
}

//-----------------------------------------------------------
int InputCallback( ImGuiInputTextCallbackData* data )
{
    textBoxState& state = *reinterpret_cast<textBoxState*>( data->UserData );
    
    switch( data->EventFlag )
    {
        case ImGuiInputTextFlags_CallbackCompletion :

            if( state.isPopupOpen && state.activeIdx != -1 )
            {
                // Tab was pressed, grab the item's text
                SetInputFromActiveIndex( data, state.activeIdx );
            }

            state.isPopupOpen       = false;
            state.activeIdx         = -1;
            state.clickedIdx        = -1;

        break;

        case ImGuiInputTextFlags_CallbackHistory :

            state.isPopupOpen = true;

            if( data->EventKey == ImGuiKey_UpArrow && state.activeIdx > 0 )
            {
                state.activeIdx--;
                state.selectionChanged = true;
            }
            else if( data->EventKey == ImGuiKey_DownArrow && state.activeIdx < state.entries.size() )
            {
                state.activeIdx++;
                state.selectionChanged = true;
            }

        break;

        case ImGuiInputTextFlags_CallbackAlways:

            if( state.clickedIdx != -1 )
            {
                // The user has clicked an item, grab the item text
                SetInputFromActiveIndex( data, state.clickedIdx );

                // Hide the popup
                state.isPopupOpen = false;
                state.activeIdx   = -1;
                state.clickedIdx  = -1;
            }

        break;

        case ImGuiInputTextFlags_CallbackCharFilter:
          state.isPopupOpen = true;
          state.activeIdx = 0;
          state.clickedIdx = -1;

        break;
    }

    return 0;
}

void drawTextBox(textBoxState& state, bool& isFocused, char * name, char * hint, char * textBuffer, int bufferSize, bool &submitted) {
  ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue    |
                                    ImGuiInputTextFlags_CallbackAlways      |
                                    ImGuiInputTextFlags_CallbackCharFilter  |
                                    ImGuiInputTextFlags_CallbackCompletion  |
                                    ImGuiInputTextFlags_CallbackHistory;

  if (ImGui::InputTextWithHint(name, hint, textBuffer, bufferSize, flags, InputCallback, &state)) {
    ImGui::SetKeyboardFocusHere( -1 );
    submitted = true;
    if( state.isPopupOpen && state.activeIdx != -1 ) {
      // This means that enter was pressed whilst a
      // the popup was open and we had an 'active' item.
      // So we copy the entry to the input buffer here
      /*
      const char*  entry  = state.entries[state.activeIdx].c_str();
      const size_t length = strlen( entry );
      memmove( textBuffer, entry, length + 1 );
      */
    } else {
      // Handle text input here
      memset(textBuffer, '\0', sizeof(textBuffer));
    }
  }

  if( state.clickedIdx != -1 ) {
    ImGui::SetKeyboardFocusHere( -1 );
    state.isPopupOpen = false;
  }

  // Get input box position, so we can place the popup under it
  state.popupPos = ImGui::GetItemRectMin();

  // Based on Omar's developer console demo: Retain focus on the input box
  if( ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
    !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked( 0 ) ) {
      ImGui::SetKeyboardFocusHere( -1 );
  }

  // Grab the position for the popup
  state.popupSize = ImVec2( ImGui::GetItemRectSize().x-60, ImGui::GetTextLineHeightWithSpacing() * 4 );
  state.popupPos.y += ImGui::GetItemRectSize().y;
  isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow);
}

void drawPopup( textBoxState& state, bool& isFocused )
{
    if( !state.isPopupOpen )
        return;

    ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0 );
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar          |
        ImGuiWindowFlags_NoResize            |
        ImGuiWindowFlags_NoMove              |
        ImGuiWindowFlags_HorizontalScrollbar |
        ImGuiWindowFlags_NoFocusOnAppearing  |
        ImGuiWindowFlags_NoSavedSettings;



    ImGui::BeginChild("myID", state.popupSize, true, flags);
    ImGui::PushAllowKeyboardFocus( false );

    for( int i = 0; i < state.entries.size(); i++ )
    {
        // Track if we're drawing the active index so we
        // can scroll to it if it has changed
        bool isIndexActive = state.activeIdx == i;

        if( isIndexActive )
        {
            // Draw the currently 'active' item differently
            // ( used appropriate colors for your own style )
            ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 1, 0, 0, 1 ) );
        }

        ImGui::PushID( i );
        if( ImGui::Selectable( state.entries[i].c_str(), isIndexActive ) )
        {
            // And item was clicked, notify the input
            // callback so that it can modify the input buffer
            state.clickedIdx = i;
        }
        ImGui::PopID();

        if( isIndexActive )
        {
            if( state.selectionChanged )
            {
                // Make sure we bring the currently 'active' item into view.
                ImGui::SetScrollHereY();
                state.selectionChanged = false;
            }

            ImGui::PopStyleColor(1);
        }
    }

    isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow);

    ImGui::PopAllowKeyboardFocus();
    ImGui::EndChild();
    ImGui::PopStyleVar(1);
}
