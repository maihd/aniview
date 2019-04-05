#ifndef __IMGUI_EXTENSIONS_H__
#define __IMGUI_EXTENSIONS_H__

#include "../imgui/imgui.h"

namespace ImGui
{
	IMGUI_API bool FileInput(const char* title, char* buffer, int length, const char* filter = "All | *.*\0*.*\0");
}

#endif /* __IMGUI_EXTENSIONS_H__ */