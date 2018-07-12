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

#include "IconFontsAwesome5.h"

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

    SDL_Window* window = SDL_CreateWindow("SpineView", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
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

        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        projMatrix = ortho(-width * 0.5f, width * 0.5f, 0, height, -10, 10);

        ImGuiContext* context = ImGui::CreateContext();
        (void)context;

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontDefault();

        ImFontConfig fontConfig;
        fontConfig.MergeMode = true;
        fontConfig.GlyphMinAdvanceX = 13.0f;
        static const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        io.Fonts->AddFontFromFileTTF("../../res/Fonts/fa-solid-900.ttf", 13.0f, &fontConfig, iconRanges);

        ImGuiImpl::Init(window);

        SpineAnimation::Create(spineAnimation, "../../res/spineboy.atlas", "../../res/spineboy.json");
        SpineAnimation::Play(spineAnimation, 0);

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

        const char* renderer = (const char*)glGetString(GL_RENDERER);
        ImGui::Text("GPU: %s", renderer);
    #endif

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0;
        style.WindowBorderSize = 0;

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
        ImGui::SetNextWindowPos(vec2(0, 0));
        ImGui::SetNextWindowSize(vec2(360, 720));
        if (ImGui::Begin("Menu", NULL, flags))
        {
            static int  index = 0;
            static char buffer[32][1024] = {};
            static char* names[32];

            if (names[0] == 0)
            {
                for (int i = 0; i < 32; i++)
                {
                    names[i] = buffer[i];
                }
            }

            SpineAnimation::AnimationNames(spineAnimation, names, 32);
            if (ImGui::Combo("Animation", &index, names, SpineAnimation::AnimationCount(spineAnimation)))
            {
                SpineAnimation::Play(spineAnimation, names[index]);
            }

            static char atlasPath[1024] = "../../res/spineboy.atlas";
            ImGui::FileDialog("Atlas path", atlasPath, sizeof(atlasPath), "Atlas\0*.atlas\0");

            static char jsonPath[1024] = "../../res/spineboy.json";
            ImGui::FileDialog("Json path", jsonPath, sizeof(jsonPath), "Json\0*.json\0");

            if (ImGui::Button("Change File"))
            {
                SpineAnimation temp;
                if (SpineAnimation::Create(temp, atlasPath, jsonPath))
                {
                    SpineAnimation::Delete(spineAnimation);

                    spineAnimation = temp;
                    SpineAnimation::Play(spineAnimation, "idle");
                }
            }
        }
        ImGui::End();

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