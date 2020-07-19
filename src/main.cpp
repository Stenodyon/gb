#include <cstdio>

#include <SDL2/SDL.h>

#include "Cart.hpp"
#include "Emulator.hpp"

u8* load_file(const char* filename)
{
    auto file = fopen(filename, "rb");
    if (!file) {
        perror(filename);
        exit(-1);
    }

    fseek(file, 0, SEEK_END);
    auto size = ftell(file);
    fseek(file, 0, SEEK_SET);

    auto* data = (u8*)malloc(size);
    if (!fread(data, size, 1, file)) {
        perror(filename);
        fclose(file);
        exit(-1);
    }

    return data;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <rom-file>\n", argv[0]);
        exit(-1);
    }

    auto filename = argv[1];
    auto rom_data = load_file(filename);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }

    auto* window = SDL_CreateWindow(
            "GUEMBOI",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            160 * 4, 144 * 4,
            SDL_WINDOW_SHOWN
            );
    if (!window) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        exit(-1);
    }

    auto* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        exit(-1);
    }

    auto* texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGB888,
            SDL_TEXTUREACCESS_STREAMING,
            160, 144
            );
    if (!texture) {
        fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        exit(-1);
    }

    GB::Cart cart(rom_data);
    GB::Emulator emulator(&cart, texture);

    while (true) {
        auto start_millis = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    goto main_loop_exit;
            }
        }

        emulator.exec_to_next_frame();

        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        auto delta_millis = SDL_GetTicks() - start_millis;
        if (delta_millis < 16)
            SDL_Delay(16 - delta_millis);
    }
main_loop_exit:

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    free(rom_data);

    return 0;
}
