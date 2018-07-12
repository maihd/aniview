#include "ImGuiImpl.h"
#include "ImGuiExtensions.h"

#include "../imgui/imgui.h"

#include <stdio.h>
#include <Windows.h>

#include "../IconFontsAwesome5.h"

namespace ImGui
{
	bool FileDialog(const char* title, char* buffer, int length, const char* filter)
	{
		bool result = false;
        
        char buttonLabel[1024];
        sprintf(buttonLabel, ICON_FA_SEARCH "##%s", title);
        if (ImGui::Button(buttonLabel, vec2(30.0f, 0.0f)))
		{
			OPENFILENAMEA ofn;
            HWND hwnd = ImGuiImpl::GetAttachHWND();

            buffer[0] = 0;

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = buffer;
            ofn.nMaxFile  = length;
            ofn.lpstrFilter = filter;
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileNameA(&ofn))
            {
                result = true;
            }
            else
            {
                result = false;
            }
		}
        
        ImGui::SameLine(ImGui::GetStyle().WindowPadding.x + 30.0f);
		result |= ImGui::InputText(title, buffer, length);
		return result;
	}
}