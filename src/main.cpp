#include <cstdio>
#include <optional>

#include "SDL.h"

#include "Cart.hpp"
#include "Emulator.hpp"
#include "Joypad.hpp"

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

void handle_keypress(GB::Emulator& emulator, SDL_KeyboardEvent* event)
{
    bool pressed = event->type == SDL_KEYDOWN;
    switch (event->keysym.sym) {
        case SDLK_w:
            emulator
                .joypad()
                .set_button_status(GB::Joypad::Buttons::UP, pressed);
            break;
        case SDLK_s:
            emulator
                .joypad()
                .set_button_status(GB::Joypad::Buttons::DOWN, pressed);
            break;
        case SDLK_a:
            emulator
                .joypad()
                .set_button_status(GB::Joypad::Buttons::LEFT, pressed);
            break;
        case SDLK_d:
            emulator
                .joypad()
                .set_button_status(GB::Joypad::Buttons::RIGHT, pressed);
            break;
        case SDLK_j:
            emulator
                .joypad()
                .set_button_status(GB::Joypad::Buttons::B, pressed);
            break;
        case SDLK_k:
            emulator
                .joypad()
                .set_button_status(GB::Joypad::Buttons::A, pressed);
            break;
        case SDLK_t:
            emulator
                .joypad()
                .set_button_status(GB::Joypad::Buttons::SELECT, pressed);
            break;
        case SDLK_y:
            emulator
                .joypad()
                .set_button_status(GB::Joypad::Buttons::START, pressed);
            break;
    }
}

[[noreturn]] void panic_usage(const char* argv0) {
    fprintf(stderr, "Usage: %s OPTIONS <rom-file>\n", argv0);
    fprintf(stderr,
            "\n"
            "OPTIONS:\n"
            "\t--trace\ttrace the opcode execution\n"
           );
    exit(-1);
}

int main(int argc, char** argv) {
    std::optional<const char*> maybe_filename{};
    bool trace = false;

    for (int argument_index = 1; argument_index < argc; ++argument_index) {
        if (!strcmp(argv[argument_index], "--trace")) {
            trace = true;
        } else {
            if (maybe_filename)
                panic_usage(argv[0]);
            maybe_filename = argv[argument_index];
        }
    }

    if (!maybe_filename)
        panic_usage(argv[0]);

    auto filename = maybe_filename.value();
    auto rom_data = load_file(filename);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }

    auto* window = SDL_CreateWindow(
            "GUEMBOI",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            160 * 4, 144 * 4,
            SDL_WINDOW_SHOWN
            );
    if (window == NULL) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        exit(-1);
    }

    auto* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        exit(-1);
    }

    auto* texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_BGR888,
            SDL_TEXTUREACCESS_STREAMING,
            160, 144
            );
    if (texture == NULL) {
        fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        exit(-1);
    }

    GB::Cart cart(rom_data);
    GB::Emulator emulator(&cart, texture);
    emulator.enable_tracing(trace);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    bool run = false;
    while (true) {
        auto start_millis = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    {
                        auto* key_event = reinterpret_cast<SDL_KeyboardEvent*>(&event);
                        handle_keypress(emulator, key_event);

                        if (key_event->type == SDL_KEYDOWN) {
                            if (key_event->keysym.sym == SDLK_n)
                                emulator.step();
                            if (key_event->keysym.sym == SDLK_c) {
                                run = true;
                                emulator.apu().unpause();
                            }
                            if (key_event->keysym.sym == SDLK_x) {
                                run = false;
                                emulator.apu().pause();
                            }
                            if (key_event->keysym.sym == SDLK_b) {
                                printf(
                                        "joypad=%02x\n",
                                        emulator.joypad().read_register()
                                      );
                            }
                        }
                    }
                    break;

                case SDL_QUIT:
                    goto main_loop_exit;
            }
        }

        if (run)
            emulator.exec_to_next_frame();

        SDL_RenderClear(renderer);
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
