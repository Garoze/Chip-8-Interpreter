#include <chrono>
#include <thread>
#include <cstdint>
#include <iostream>

#include <SDL2/SDL.h>

#include "../include/Chip8.hpp"
#include "../include/SDLKeymap.hpp"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: chip8 <ROM>" << std::endl;
        exit(1);
    }

    CHIP8 chip8 = CHIP8();
    int width = 1024;
    int height = 512;

    SDL_Window* window = NULL;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cerr << "SDL Could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    window = SDL_CreateWindow(
            "CHIP-8 Interpreter",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width, height,
            SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        std::cerr << "Window could not be created! SDL_ERROR: " << SDL_GetError() << std::endl;
        exit(2);
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, width, height);

    SDL_Texture* texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            64, 32);

    std::uint32_t temp_pixels[2048];

load:
    if (!chip8.LoadROM(argv[1]))
    {
        std::cerr << "Could not load the rom!" << std::endl;
        exit(1);
    }

    while (true)
    {
        chip8.EmulateCycle();

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) exit(0);

            if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    exit(0);

                if (e.key.keysym.sym == SDLK_F1)
                    goto load;

                for (int i = 0; i < 16; ++i)
                    if (e.key.keysym.sym == SDLKeymap[i])
                        chip8.keypad[i] = 1;
            }

            if (e.type == SDL_KEYUP)
            {
                for (int i = 0; i < 16; ++i)
                    if (e.key.keysym.sym == SDLKeymap[i])
                        chip8.keypad[i] = 0;
            }
        }

        if (chip8.drawFlag)
        {
            chip8.drawFlag = false;

            for (int i = 0; i < 2048; ++i)
            {
               std::uint8_t pixel = chip8.display[i];
                temp_pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }

            SDL_UpdateTexture(texture, NULL, temp_pixels, 64 * sizeof(Uint32));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        std::this_thread::sleep_for(std::chrono::microseconds(1200));
    }

    return EXIT_SUCCESS;
}

