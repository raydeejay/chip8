/***********************************************************
 * CHIP8
 *
 * A CHIP-8 emulator using C and SDL
 **********************************************************/

#include <stdio.h>
#include <string.h>
#include <float.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "chip8.h"

//Screen dimension constants
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320
#define SCREEN_FPS 500
#define SCREEN_TICKS_PER_FRAME (1000 / SCREEN_FPS)

typedef enum {
    GAME
} machine_modes;

machine_modes machine_mode = GAME;

//Starts up SDL and creates window
int init();

//Frees media and shuts down SDL
void myclose();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

// Sound effects, not sure about the limit yet
Mix_Chunk *gSfx[72] = { NULL };
int gMaxSfx = -1;

/*************************************************
 ** MAIN CODE
 ************************************************/
int init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 0;
    }

    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
        printf("Warning: Linear texture filtering not enabled!");
    }

    /* if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0")) { */
    /*     printf("Warning: Linear texture filtering not enabled!"); */
    /* } */

    gWindow = SDL_CreateWindow("CHIP-8",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                SCREEN_WIDTH, SCREEN_HEIGHT,
                                SDL_WINDOW_SHOWN);

    if (gWindow == NULL) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        return 0;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        return 0;
    }

    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

    SDL_RenderSetLogicalSize(gRenderer, CHIP8_WIDTH, CHIP8_HEIGHT);

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return 0;
    }

    if (Mix_OpenAudio(22050, AUDIO_U8, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    }

    return 1;
}

// NOTE that if you create a close() function, SDL_Init() will hang and never succeed :-)
// (because you are shadowing the standard library's close())
void myclose() {
    for (int i = 0; gSfx[i]; ++i) {
        if (gSfx[i]) {
            Mix_FreeChunk(gSfx[i]);
            gSfx[i] = NULL;
        }
    }

    //Destroy window
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("You must provide a filename\n");
        return 1;
    }

    const char *filename = argv[1];
    chip8_t *machine = chip8_new();
    chip8_init(machine);
    chip8_loadFile(machine, filename);

    // Start up SDL and create window
    if (!init()) {
        printf("Failed to initialize!\n");
    }
    else {
        // Main loop flag
        int quit = 0;

        // Event handler
        SDL_Event e;
        const Uint8 *state = SDL_GetKeyboardState(NULL);

        // Start counting frames per second
        int countedFrames = 0;

        // While application is running
        while (!quit)
        {
            int startFrame = SDL_GetTicks();
            unsigned char keys[16] = { 0 };

            // NOTE that only game mode is implemented for now
            if (machine_mode == GAME) {
                // Handle events on queue
                while (SDL_PollEvent(&e) != 0) {
                    // User requests quit
                    if (e.type == SDL_QUIT) {
                        quit = 1;
                    }
                    else if (e.type == SDL_KEYDOWN) {
                        switch (e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            quit = 1;
                            break;
                        case SDLK_r:
                            if (e.key.keysym.mod & KMOD_CTRL) {
                                chip8_init(machine);
                                chip8_loadFile(machine, filename);
                            }
                            break;
                        default:
                            break;
                        }
                    }
                }

                // get keys
                if (state[SDL_SCANCODE_1]) { keys[0x1] = 1; }
                if (state[SDL_SCANCODE_2]) { keys[0x2] = 1; }
                if (state[SDL_SCANCODE_3]) { keys[0x3] = 1; }
                if (state[SDL_SCANCODE_4]) { keys[0xC] = 1; }
                
                if (state[SDL_SCANCODE_Q]) { keys[0x4] = 1; }
                if (state[SDL_SCANCODE_W]) { keys[0x5] = 1; }
                if (state[SDL_SCANCODE_E]) { keys[0x6] = 1; }
                if (state[SDL_SCANCODE_R]) { keys[0xD] = 1; }
                
                if (state[SDL_SCANCODE_A]) { keys[0x7] = 1; }
                if (state[SDL_SCANCODE_S]) { keys[0x8] = 1; }
                if (state[SDL_SCANCODE_D]) { keys[0x9] = 1; }
                if (state[SDL_SCANCODE_F]) { keys[0xE] = 1; }
                
                if (state[SDL_SCANCODE_Z]) { keys[0xA] = 1; }
                if (state[SDL_SCANCODE_X]) { keys[0x0] = 1; }
                if (state[SDL_SCANCODE_C]) { keys[0xB] = 1; }
                if (state[SDL_SCANCODE_V]) { keys[0xF] = 1; }

                chip8_setKeys(machine, keys);

                // run code
                chip8_cycle(machine);

                // refresh the display if necessary
                if (machine->redraw) {
                    // Clear screen
                    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
                    SDL_RenderClear(gRenderer);

                    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                    for (int i = 0; i < CHIP8_HEIGHT; ++i)
                        for (int j = 0; j < CHIP8_WIDTH; ++j)
                            if (machine->VRAM[i * CHIP8_WIDTH + j])
                                SDL_RenderDrawPoint(gRenderer, j, i);
                    machine->redraw = 0;
                }

            }

            // Update screen
            SDL_RenderPresent(gRenderer);
            ++countedFrames;

            // decrement timers at 60Hz

            // Throttle
            int frameTicks = SDL_GetTicks() - startFrame;
            if (frameTicks < SCREEN_TICKS_PER_FRAME) {
                SDL_Delay(SCREEN_TICKS_PER_FRAME - frameTicks);
            }
        }
    }

    // Free resources and close SDL
    chip8_destroy(machine);
    free(machine);

    myclose();

    return 0;
}
