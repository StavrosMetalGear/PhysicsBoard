#include "BoardApp.h"
#include "imgui.h"
#include "implot.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <SDL.h>
#include <SDL_opengles2.h>
#include <cstdio>

// SDL on Android expects SDL_main to be defined by the native library.
int SDL_main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        std::fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Request GLES 3.0 context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    int winW = 1280, winH = 720;
    SDL_Window* window = SDL_CreateWindow("PhysicsBoard",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        winW, winH,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    // ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    const char* glsl_version = "#version 300 es";
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Application
    BoardApp app;

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_FINGERDOWN: {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                float x = event.tfinger.x * w;
                float y = event.tfinger.y * h;
                app.handleTouchDown(ImVec2(x, y), (int)event.tfinger.fingerId);
                break;
            }
            case SDL_FINGERMOTION: {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                float x = event.tfinger.x * w;
                float y = event.tfinger.y * h;
                app.handleTouchMove(ImVec2(x, y), (int)event.tfinger.fingerId);
                break;
            }
            case SDL_FINGERUP: {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                float x = event.tfinger.x * w;
                float y = event.tfinger.y * h;
                app.handleTouchUp(ImVec2(x, y), (int)event.tfinger.fingerId);
                break;
            }
            default:
                break;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        app.render();

        ImGui::Render();
        glViewport(0, 0, winW, winH);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    if (gl_context) SDL_GL_DeleteContext(gl_context);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
