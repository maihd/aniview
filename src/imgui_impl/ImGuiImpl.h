#ifndef __IMGUI_IMPL_H__
#define __IMGUI_IMPL_H__

struct SDL_Window;
typedef union SDL_Event SDL_Event;
class ImDrawData;

namespace ImGuiImpl
{
    IMGUI_API bool        Init(SDL_Window* window);
    IMGUI_API void        Shutdown();
    IMGUI_API void        NewFrame(SDL_Window* window);
    IMGUI_API bool        ProcessEvent(SDL_Event* event); 
    IMGUI_API void        RenderDrawData(ImDrawData* draw_data);

    // Use if you want to reset your rendering device without losing ImGui state.
    IMGUI_API void        InvalidateDeviceObjects();
    IMGUI_API bool        CreateDeviceObjects();
}

#endif