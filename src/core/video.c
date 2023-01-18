#include <SDL2/SDL.h>
#include "video.h"
#include "log.h"

const int DEFAULT_WINDOW_W = 1024;
const int DEFAULT_WINDOW_H = 768;

const char *g_title = "state.io";

SDL_Window *g_Window = NULL;

SDL_Renderer *g_Renderer = NULL;

int VDO_CreateWindow() {
    Uint32 flags = SDL_WINDOW_SHOWN;
    g_Window = SDL_CreateWindow(
        g_title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        DEFAULT_WINDOW_W,
        DEFAULT_WINDOW_H,
        flags
    );
    if (g_Window == NULL) {
        LogError("Unable to create window: %s");
        return -1;
    }
    int w, h;
    SDL_GetWindowSize(g_Window, &w, &h);
    LogInfo("Window %dx%d", w, h);
    return 0;
}

int VDO_CreateRenderer() {
    if (g_Window == NULL) {
        LogError("Unable to create renderer, window not available: %s");
        return -1;
    }
    Uint32 flags = SDL_RENDERER_ACCELERATED;
    g_Renderer = SDL_CreateRenderer(
        g_Window,
        -1,
        flags
    );
    if (g_Renderer == NULL) {
        LogError("Unable to create renderer: %s");
        return -1;
    }
    return 0;
}

int VDO_Init() {
    if (VDO_CreateWindow() != 0)
        return -1;
    if (VDO_CreateRenderer() != 0)
        return -1;
    return 0;
}

void VDO_Quit() {
    SDL_DestroyRenderer(g_Renderer);
    SDL_DestroyWindow(g_Window);
    g_Renderer = NULL;
    g_Window = NULL;
}

SDL_Window * VDO_GetWindow() {
    return g_Window;
}

void VDO_GetWindowSize(int *w, int *h) {
    SDL_GetWindowSize(g_Window, w, h);
}

SDL_Renderer * VDO_GetRenderer() {
    return g_Renderer;
}

int VDO_GetFPS() {
    return 60;
}