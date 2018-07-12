#ifndef __IMGUI_IMPL_H__
#define __IMGUI_IMPL_H__

#include <Windows.h>
#include "ImGuiExtensions.h"

struct SDL_Window;
struct ImDrawData;
typedef union SDL_Event SDL_Event;

namespace ImGuiImpl
{
    IMGUI_API bool        Init(SDL_Window* window);
    IMGUI_API void        Shutdown();
    IMGUI_API void        NewFrame(SDL_Window* window, float deltaTime);
    IMGUI_API bool        ProcessEvent(const SDL_Event* event); 
    IMGUI_API void        RenderDrawData(ImDrawData* draw_data);

    // Use if you want to reset your rendering device without losing ImGui state.
    IMGUI_API void        InvalidateDeviceObjects();
    IMGUI_API bool        CreateDeviceObjects();

	IMGUI_API HWND        GetAttachHWND(void);
}

#endif