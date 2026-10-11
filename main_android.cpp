#include "BoardApp.h"
#include "imgui.h"
#include "implot.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <SDL.h>
#include <SDL_opengles2.h>
#include <cstdio>
#include <unordered_map>


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
    // Map to track active fingers for gesture recognition
    std::unordered_map<SDL_FingerID, ImVec2> fingers;
    float prevDistance = 0.0f;
    ImVec2 prevCenter = {0,0};
    bool sdlTextActive = false;
    SDL_Rect sdlTextRect = {0,0,0,0};

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
                ImVec2 pos = { event.tfinger.x * w, event.tfinger.y * h };
                // If Text tool is active, handle IME start/stop
                if (app.getCurrentTool() == ToolType::Text) {
                    // If text input not active, start IME at touch position
                    if (!sdlTextActive) {
                        SDL_StartTextInput();
                        sdlTextRect.x = static_cast<int>(pos.x);
                        sdlTextRect.y = static_cast<int>(pos.y);
                        sdlTextRect.w = 300; sdlTextRect.h = 80;
                        SDL_SetTextInputRect(&sdlTextRect);
                        app.startTextInputAt(pos);
                        sdlTextActive = true;
                    } else {
                        // If already active, touching again commits text (if outside rect)
                        SDL_Rect r = sdlTextRect;
                        if (!(static_cast<int>(pos.x) >= r.x && static_cast<int>(pos.x) <= r.x + r.w
                              && static_cast<int>(pos.y) >= r.y && static_cast<int>(pos.y) <= r.y + r.h)) {
                            SDL_StopTextInput();
                            app.stopTextInput(true);
                            sdlTextActive = false;
                        }
                    }
                    // Do not add to fingers or start drawing when Text tool handled IME
                    break;
                }
                fingers[event.tfinger.fingerId] = pos;
                // If single touch, forward to app as touch down
                if (fingers.size() == 1) {
                    app.handleTouchDown(pos, (int)event.tfinger.fingerId);
                }
                // If two fingers, initialize gesture state
                if (fingers.size() >= 2) {
                    auto it = fingers.begin();
                    ImVec2 a = it->second; ++it; ImVec2 b = it->second;
                    prevCenter = { (a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f };
                    float dx = b.x - a.x; float dy = b.y - a.y;
                    prevDistance = std::sqrt(dx*dx + dy*dy);
                }
                break;
            }
            case SDL_FINGERMOTION: {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                ImVec2 pos = { event.tfinger.x * w, event.tfinger.y * h };
                fingers[event.tfinger.fingerId] = pos;

                if (fingers.size() == 1) {
                    // Single-finger drag -> drawing or panning depending on tool state
                    app.handleTouchMove(pos, (int)event.tfinger.fingerId);
                }
                else if (fingers.size() >= 2) {
                    // Two-finger gesture: compute new center and distance
                    auto it = fingers.begin();
                    ImVec2 a = it->second; ++it; ImVec2 b = it->second;
                    ImVec2 center = { (a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f };
                    float dx = b.x - a.x; float dy = b.y - a.y;
                    float dist = std::sqrt(dx*dx + dy*dy);
                    if (prevDistance > 0.0f) {
                        float scale = dist / prevDistance;
                        // Zoom around center
                        app.scaleZoom(scale, center);
                        // Pan by movement of center
                        ImVec2 delta = { center.x - prevCenter.x, center.y - prevCenter.y };
                        app.addPan(delta.x, delta.y);
                    }
                    prevCenter = center;
                    prevDistance = dist;
                }
                break;
            }
            case SDL_FINGERUP: {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                ImVec2 pos = { event.tfinger.x * w, event.tfinger.y * h };
                // If Text IME active and this was a tap to commit, ignore fingerup drawing
                if (sdlTextActive && app.getCurrentTool() == ToolType::Text) {
                    // Do nothing on finger up; text committed via explicit stop or external IME action
                } else {
                    // If the finger removed was the only one, finish drawing
                    if (fingers.size() == 1) {
                        app.handleTouchUp(pos, (int)event.tfinger.fingerId);
                    }
                }
                // Erase finger
                fingers.erase(event.tfinger.fingerId);
                // Reset gesture state when fewer than two fingers remain
                if (fingers.size() < 2) {
                    prevDistance = 0.0f;
                }
                break;
            }
            case SDL_TEXTINPUT: {
                // Append incoming UTF-8 text to the application buffer
                app.appendTextInput(event.text.text);
                break;
            }
            case SDL_TEXTEDITING: {
                // Composition text can be handled or ignored (we append final TEXTINPUT events)
                break;
            }
            case SDL_KEYDOWN: {
                if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                    if (sdlTextActive) {
                        SDL_StopTextInput();
                        app.stopTextInput(true);
                        sdlTextActive = false;
                    }
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (sdlTextActive) {
                        SDL_StopTextInput();
                        app.stopTextInput(false);
                        sdlTextActive = false;
                    }
                }
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
