module;

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

export module MapleGarlicEngine:AssetManager;

import std;

export SDL_Surface* LoadImage(const std::string& filename);

export TTF_Font* LoadFont(const std::string& filename, const float size);
