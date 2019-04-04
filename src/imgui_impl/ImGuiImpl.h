#ifndef __IMGUI_IMPL_H__
#define __IMGUI_IMPL_H__

#include "ImGuiExtensions.h"

struct Window;
struct ImDrawData;
struct WindowEvent;

namespace ImGuiImpl
{
    IMGUI_API bool        Init(Window* window);
    IMGUI_API void        Shutdown();
    IMGUI_API void        NewFrame(Window* window, float deltaTime);
    IMGUI_API bool        ProcessEvent(const WindowEvent* event); 
    IMGUI_API void        RenderDrawData(ImDrawData* draw_data);

    // Use if you want to reset your rendering device without losing ImGui state.
    IMGUI_API void        InvalidateDeviceObjects();
    IMGUI_API bool        CreateDeviceObjects();

	IMGUI_API void*       GetAttachHWND(void);
}

#endif