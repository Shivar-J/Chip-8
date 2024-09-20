#include <iostream>
#include <chrono>
#include <thread>
#include <stdint.h>
#include <SDL.h>
#include "cpu.hpp"

uint8_t keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

void audio_callback(void* userdata, uint8_t* stream, int len) {
    for (int i = 0; i < len; i++)
        stream[i] = (i / 128) % 2 == 0 ? 127 : -128;
}

int main(int argc, char* args[]) {
    if (argc != 2)
        return 1;

    Chip8 chip8 = Chip8();
    int width = 1024;
    int height = 512;

    SDL_Window* window = NULL;

    if (SDL_Init(SDL_INIT_EVERYTHING))
        exit(1);

    window = SDL_CreateWindow(
        "Chip-8 Emulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_SHOWN
    );

    if (window == NULL)
        exit(2);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, width, height);

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    int video_pitch = sizeof(chip8.video[0]) * 64;

    SDL_AudioSpec wav_spec;
    wav_spec.freq = 44100;
    wav_spec.format = AUDIO_S8;
    wav_spec.channels = 1;
    wav_spec.samples = 2048;
    wav_spec.callback = audio_callback;

    if (SDL_OpenAudio(&wav_spec, NULL) < 0)
        return 1;

    auto last_cycle = std::chrono::high_resolution_clock::now();

    chip8.LoadROM(args[1]);

    while (true) {
        SDL_Event e;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                exit(0);

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    exit(0);

                for (int i = 0; i < 16; i++) {
                    if (e.key.keysym.sym == keymap[i])
                        chip8.keypad[i] = 1;
                }
            }

            if (e.type == SDL_KEYUP) {
                for (int i = 0; i < 16; i++) {
                    if (e.key.keysym.sym == keymap[i])
                        chip8.keypad[i] = 0;
                }
            }
        }

        auto curr_cycle = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(curr_cycle - last_cycle).count();

        if (dt > 3) {
            if (chip8.get_soundtimer() > 0)
                SDL_PauseAudio(0);
            else
                SDL_PauseAudio(1);

            last_cycle = curr_cycle;

            chip8.cycle();

            SDL_UpdateTexture(texture, nullptr, chip8.video, video_pitch);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}