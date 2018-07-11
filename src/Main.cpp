#include <stdio.h>

#define TIMER_IMPL
#include "Timer.h"
#include "vmath/vmath.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "imgui/imgui.h"
#include "imgui_impl/ImGuiImpl.h"

#define MEMBUF_IMPL
#include "memwise/membuf.h"

#include "System.h"
#include "Shader.h"
#include "Texture.h"
#include "SpineAnimation.h"

#undef main
int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        System::Error("SDL_Init(): %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SpineView", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (!window)
    {
        System::Error("SDL_CreateWindow(): %s", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext)
    {
        System::Error("SDL_GL_CreateContext(): %s", SDL_GetError());
        return 1;
    }

    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK)
    {
        System::Error("glewInit(): %s", glewGetErrorString(glewStatus));
        return 1;
    }

    SpineAnimation spineAnimation;
    SpineAnimation::Create(spineAnimation, "../../res/spineboy.atlas", "../../res/spineboy.json");

    Shader defaultShader;
    Shader::Load("../../res/Shaders/Default", &defaultShader);

    ImGuiContext* context = ImGui::CreateContext();
    (void)context;

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    Timer timer;
    float deltaTime = 0.0f;
    SDL_Event event;

    while (true)
    {
        Timer::NewFrame(timer);

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                break;
            }
            else
            {
                ImGuiImpl::ProcessEvent(&event);
            }
        }
        if (event.type == SDL_QUIT)
        {
            break;
        }

        SpineAnimation::Update(spineAnimation, deltaTime);

        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGuiImpl::NewFrame(window, deltaTime);
        
        ImGui::Button("Click me!");
        
        ImGui::Render();
        ImGuiImpl::RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);

        Timer::EndFrame(timer);
        Timer::Sleep(timer);

        deltaTime = (float)Timer::Seconds(timer);
    }
    

    SpineAnimation::Delete(spineAnimation);

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}