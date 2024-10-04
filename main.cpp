#include <Windows.h>
#include <chrono>
#include <thread>
#include <stdint.h>
#include <SDL.h>
#include "cpu.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

uint8_t keymap[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f, SDLK_v,
};

void audio_callback(void* userdata, uint8_t* stream, int len) {
    for (int i = 0; i < len; i++)
        stream[i] = (i / 128) % 2 == 0 ? 127 : -128;
}

int main(int argc, char* args[]) {
    ShowWindow(GetConsoleWindow(), false);

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    int width = 1024;
    int height = 512;
    SDL_Window* window = SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    SDL_RenderSetLogicalSize(renderer, width, height);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (!texture) {
        std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_AudioSpec wav_spec;
    wav_spec.freq = 44100;
    wav_spec.format = AUDIO_S8;
    wav_spec.channels = 1;
    wav_spec.samples = 2048;
    wav_spec.callback = audio_callback;

    if (SDL_OpenAudio(&wav_spec, NULL) < 0) {
        std::cerr << "Audio could not be opened! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(texture);
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    Chip8 chip8;
    std::vector<std::string> instructions;
    chip8.LoadROM("tetris.ch8");

    auto last_cycle = std::chrono::high_resolution_clock::now();
    bool running = true;

    std::vector<std::string> register_info;

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);

            if (e.type == SDL_QUIT) {
                running = false;
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                for (int i = 0; i < 16; i++) {
                    if (e.key.keysym.sym == keymap[i]) {
                        chip8.keypad[i] = 1;
                    }
                }
            }
            if (e.type == SDL_KEYUP) {
                for (int i = 0; i < 16; i++) {
                    if (e.key.keysym.sym == keymap[i]) {
                        chip8.keypad[i] = 0;
                    }
                }
            }
        }

        auto curr_cycle = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(curr_cycle - last_cycle).count();

        if (dt > 3) {
            last_cycle = curr_cycle;

            if (chip8.get_soundtimer() > 0) {
                SDL_PauseAudio(0);
            }
            else {
                SDL_PauseAudio(1);
            }
             
            chip8.cycle(instructions);
            chip8.print_registers(register_info);
            
            SDL_UpdateTexture(texture, nullptr, chip8.video, 64 * sizeof(uint32_t));

            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::Begin("Chip-8 Emulator", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

            ImGui::BeginChild("Instructions", ImVec2(150, height), true);
            for (int i = 0; i < instructions.size(); i++)
                ImGui::Text("%s", instructions[i].c_str());
            if (ImGui::GetScrollY() + ImGui::GetWindowHeight() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("Video", ImVec2(660, 340), true);
            ImGui::Image(texture, ImVec2(640, 320));
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("Registers", ImVec2(200, height), true);
            for (int i = 0; i < register_info.size(); i++)
                ImGui::Text("%s", register_info[i].c_str());
            ImGui::EndChild();
            ImGui::End();
            ImGui::Render();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
            SDL_RenderPresent(renderer);

        }
    }

    instructions.clear();
    register_info.clear();
    SDL_DestroyTexture(texture);
    SDL_CloseAudio();
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
