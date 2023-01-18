#ifndef _LOG_H
#define _LOG_H

#include <SDL2/SDL_log.h>

#define LogInfo(...) SDL_LogInfo(                             \
    SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__                 \
)
#define LogError(...) SDL_LogError(                           \
    SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__, SDL_GetError() \
)

#endif /* _LOG_H */