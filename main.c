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
#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 400
#define GAL_WIDTH 64
#define GAL_HEIGHT 32
#define SCREEN_FPS 60
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
Mix_Chunk *gSfx[72] = {};
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

    SDL_RenderSetLogicalSize(gRenderer, GAL_WIDTH, GAL_HEIGHT);

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
        unsigned short keys = 0;

        // Event handler
        SDL_Event e;
        const Uint8 *state = SDL_GetKeyboardState(NULL);

        // Start counting frames per second
        int countedFrames = 0;
        
        // While application is running
        while (!quit)
        {
            int startFrame = SDL_GetTicks();
            keys = 0;

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
                if (state[SDL_SCANCODE_1]) { keys |= (1 <<  0); }
                if (state[SDL_SCANCODE_2]) { keys |= (1 <<  1); }
                if (state[SDL_SCANCODE_3]) { keys |= (1 <<  2); }
                if (state[SDL_SCANCODE_4]) { keys |= (1 <<  3); }
                if (state[SDL_SCANCODE_Q]) { keys |= (1 <<  4); }
                if (state[SDL_SCANCODE_W]) { keys |= (1 <<  5); }
                if (state[SDL_SCANCODE_E]) { keys |= (1 <<  6); }
                if (state[SDL_SCANCODE_R]) { keys |= (1 <<  7); }
                if (state[SDL_SCANCODE_A]) { keys |= (1 <<  8); }
                if (state[SDL_SCANCODE_S]) { keys |= (1 <<  9); }
                if (state[SDL_SCANCODE_D]) { keys |= (1 << 10); }
                if (state[SDL_SCANCODE_F]) { keys |= (1 << 11); }
                if (state[SDL_SCANCODE_Z]) { keys |= (1 << 12); }
                if (state[SDL_SCANCODE_X]) { keys |= (1 << 13); }
                if (state[SDL_SCANCODE_C]) { keys |= (1 << 14); }
                if (state[SDL_SCANCODE_V]) { keys |= (1 << 15); }

                chip8_setKeys(machine, keys);

                // Clear screen
                SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
                SDL_RenderClear(gRenderer);

                // refresh the display if necessary
                if (machine->redraw) {
                    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                    for (int i = 0; i < GAL_HEIGHT; ++i)
                        for (int j = 0; j < GAL_WIDTH; ++j)
                            if (machine->VRAM[i * GAL_WIDTH + j])
                                SDL_RenderDrawPoint(gRenderer, j, i);
                    machine->redraw = 0;
                }

                // run code
                chip8_cycle(machine);
            }

            // Update screen
            SDL_RenderPresent(gRenderer);
            ++countedFrames;

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
