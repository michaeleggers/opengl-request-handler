#include "utils.h"

#include <SDL3/SDL.h>

#include <string>

std::string GetExePath()
{
    const char* path = SDL_GetBasePath();

    return std::string(path);
}
