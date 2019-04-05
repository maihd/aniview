#include <GL/glew.h>
#include <sora/window.h>

#include "imgui/imgui.h"
#include "imgui_impl/ImGuiImpl.h"

#include "System.h"
#include "Shader.h"
#include "Texture.h"
#include "SpineAnimation.h"

#include "IconFontsAwesome5.h"

// Request mobile devices (laptop, surface, ...) use integrated GPU 
extern "C"
{
    __declspec(dllexport) int NvOptimusEnablement                  = 0x1;
    __declspec(dllexport) int AmdPowerXpressRequireHighPerformance = 0x1;
}

namespace Engine
{
    void Init(Window* window);
    void Quit(void);

    void OnGUI(float deltaTime);
    void Render(void);
    void Update(float deltaTime, float fixedDeltaTime);
}

int main(int argc, char* argv[])
{
    Window* window = Window::create("Aniview", 1280, 720, WindowFlags::Visible | WindowFlags::OpenGL);
    if (!window || !window->make_current())
    {
        //System::Error("SDL_CreateWindow(): %s", SDL_GetError());
        return 1;
    }

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GLContext glContext = SDL_GL_CreateContext(window);
    //if (!glContext)
    //{
    //    System::Error("SDL_GL_CreateContext(): %s", SDL_GetError());
    //    return 1;
    //}
    
    Engine::Init(window);

    WindowEvent event;

    long  ticks       = performance::now();
    long  delta_ticks = performance::now();
    long  limit_ticks = performance::frequency() / 60;
    float deltaTime = 0.0f;
    float totalTime = 0.0f;
    float fixedStepTimer = 0.0f;
    float fixedDeltaTime = 1.0f / 40; // Animation run in 40fps
    while (true)
    {
        delta_ticks = performance::now() - ticks;
        if (delta_ticks < limit_ticks)
        {
            double sleep_time = double(limit_ticks - delta_ticks) / (performance::frequency());
            delta_ticks = limit_ticks;
            performance::usleep((long)(sleep_time * 1000 * 1000));
        }
        ticks = ticks + delta_ticks;

        deltaTime = float(delta_ticks) / (performance::frequency());
        totalTime = totalTime + deltaTime;
        fixedStepTimer += deltaTime;

        while (WindowEvent::poll_event(&event))
        {
            if (event.type == WindowEvent::Quit)
            {
                break;
            }
            else
            {
                ImGuiImpl::ProcessEvent(&event);
            }
        }
        if (event.type == WindowEvent::Quit)
        {
            break;
        }

        if (fixedStepTimer >= fixedDeltaTime)
        {
            fixedStepTimer = math::fmod(fixedStepTimer, fixedDeltaTime);
            Engine::Update(deltaTime, fixedDeltaTime);
        }
        else
        {
            Engine::Update(deltaTime, 0.0f);
        }

        Engine::OnGUI(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT);
        Engine::Render();
        window->swap_buffers();
    }
    
    Engine::Quit();

    Window::destroy(window);
    return 0;
}

namespace Engine
{
    Mesh mesh;
    Shader defaultShader;
    SpineAnimation spineAnimation;
    Window* window;
    
    float4x4 projMatrix;

    void Init(Window* window)
    {
        Engine::window = window;

        int width, height;
        width = window->width;
        height = window->height;

        glViewport(0, 0, width, height);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        projMatrix = float4x4::ortho(-width, width, -height, height, -10, 10);

        (void)ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontDefault();

        ImFontConfig fontConfig;
        fontConfig.MergeMode = true;
        fontConfig.GlyphMinAdvanceX = 13.0f;
        static const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        io.Fonts->AddFontFromFileTTF("../../../res/Fonts/fa-solid-900.ttf", 13.0f, &fontConfig, iconRanges);

        ImGuiImpl::Init(window);

        SpineAnimation::Create(spineAnimation, "../../../res/spineboy.atlas", "../../../res/spineboy.json");
        SpineAnimation::Play(spineAnimation, 0);

        Mesh::Create(mesh);
        Shader::Load("../../../res/Shaders/Default", &defaultShader);
    }

    void Quit(void)
    {
        SpineAnimation::Delete(spineAnimation);
    }

    void OnGUI(float deltaTime)
    {
        ImGuiImpl::NewFrame(window, deltaTime);

    #if !defined(NDEBUG)
        ImGui::Text("Debug information");
        ImGui::Text("FPS: %f", 1.0f / deltaTime);

        const char* renderer = (const char*)glGetString(GL_RENDERER);
        ImGui::Text("GPU: %s", renderer);
    #endif

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0;
        style.WindowBorderSize = 0;

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(360, 720));
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

            static char atlasPath[1024] = "../../../res/spineboy.atlas";
            ImGui::FileInput("Atlas path", atlasPath, sizeof(atlasPath), "Atlas | *.atlas\0*.atlas\0");

            static char jsonPath[1024] = "../../../res/spineboy.json";
            ImGui::FileInput("Json path", jsonPath, sizeof(jsonPath), "Json | *.json\0*.json\0");
            {
                //int last_index = string::last_index_of(jsonPath, '.');
                //if (last_index && strn)
            }

            if (ImGui::Button("Change File"))
            {
                SpineAnimation temp;
                if (SpineAnimation::Create(temp, atlasPath, jsonPath))
                {
                    SpineAnimation::Delete(spineAnimation);

                    spineAnimation = temp;
                    SpineAnimation::Play(spineAnimation, 0);
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