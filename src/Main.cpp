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

// Request mobile devices (laptop, surface, ...) use integrated GPU 
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement                  = 0x1;
    __declspec(dllexport) int   AmdPowerXpressRequireHighPerformance = 0x1;
}

namespace Engine
{
    void Init(SDL_Window* window);
    void Quit(void);

    void OnGUI(float deltaTime);
    void Render(void);
    void Update(float deltaTime, float fixedDeltaTime);
}

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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
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
    
    Engine::Init(window);

    Timer timer;
    SDL_Event event;

    float deltaTime = 0.0f;
    float totalTime = 0.0f;
    float fixedStepTimer = 0.0f;
    float fixedDeltaTime = 1.0f / 40; // Animation run in 40fps
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

        if (fixedStepTimer >= fixedDeltaTime)
        {
            fixedStepTimer = fmodf(fixedStepTimer, fixedDeltaTime);
            Engine::Update(deltaTime, fixedDeltaTime);
        }
        else
        {
            Engine::Update(deltaTime, 0.0f);
        }

        Engine::OnGUI(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT);
        Engine::Render();
        SDL_GL_SwapWindow(window);

        Timer::EndFrame(timer);
        Timer::Sleep(timer);

        deltaTime = (float)Timer::Seconds(timer);
        totalTime = totalTime + deltaTime;
        fixedStepTimer += deltaTime;
    }
    
    Engine::Quit();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

namespace Engine
{
    Mesh mesh;
    Shader defaultShader;
    SpineAnimation spineAnimation;
    SDL_Window* window;
    
    mat4 projMatrix;

    void Init(SDL_Window* window)
    {
        Engine::window = window;

        glViewport(0, 0, 800, 600);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        projMatrix = ortho(-400, 400, 0, 600, -10, 10);

        ImGuiContext* context = ImGui::CreateContext();
        (void)context;

        ImGuiIO& io = ImGui::GetIO();
        (void)io;

        ImGuiImpl::Init(window);

        SpineAnimation::Create(spineAnimation, "../../res/spineboy.atlas", "../../res/spineboy.json");
        SpineAnimation::Play(spineAnimation, "run");

        Mesh::Create(mesh);
        Shader::Load("../../res/Shaders/Default", &defaultShader);
    }

    void Quit(void)
    {
        SpineAnimation::Delete(spineAnimation);
    }

    void OnGUI(float deltaTime)
    {
        ImGuiImpl::NewFrame(window, deltaTime);

    #if 1
        ImGui::Text("Debug information");
        ImGui::Text("FPS: %f", 1.0f / deltaTime);
    #endif

        ImGui::Render();
    }

    void Render(void)
    {
        SpineAnimation::Render(spineAnimation, mesh, defaultShader, projMatrix);
        ImGuiImpl::RenderDrawData(ImGui::GetDrawData());
    }

    void Update(float deltaTime, float fixedDeltaTime)
    {
        SpineAnimation::Update(spineAnimation, fixedDeltaTime);
    }
}