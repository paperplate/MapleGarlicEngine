module;
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cassert>

module MapleGarlicEngine;
import :AssetManager;
import :Utils;

import std;

// Source - https://stackoverflow.com/a/4119881
// Posted by Timmmm, modified by community. See post 'Timeline' for change history
// Retrieved 2026-07-07, License - CC BY-SA 4.0

bool ichar_equals(char a, char b)
{
   return std::tolower(static_cast<unsigned char>(a)) ==
          std::tolower(static_cast<unsigned char>(b));
}

bool CaseInsensitiveCompare(std::string_view lhs, std::string_view rhs)
{
   return std::ranges::equal(lhs, rhs, ichar_equals);
}

SDL_Surface* LoadImage(const std::string& filename)
{
   SDL_Surface* surface = nullptr;

   std::filesystem::path p(filename);
   if (std::filesystem::exists(p))
   {
      if (CaseInsensitiveCompare(p.extension().string(), ".bmp"))
      {
         surface = SDL_LoadBMP(p.c_str());
      }
      else if (CaseInsensitiveCompare(p.extension().string(), ".png"))
      {
         surface = IMG_Load(p.c_str());
      }
      else
      {
         log(LogType::ERROR, std::string("Image file type not supported: ") + p.string());
         assert(0 && "Image file type not supported");
      }
   }
   else
   {
      log(LogType::ERROR, std::string("Image file not found: ") + p.string());
      assert(0 && "Image file not found");
   }

   return surface;
}

TTF_Font* LoadFont(const std::string& filename, const float size)
{
   TTF_Font* font = nullptr;

   std::filesystem::path p(filename);
   if (std::filesystem::exists(p))
   {
      font = TTF_OpenFont(p.c_str(), size);
   }
   else
   {
      log(LogType::ERROR, std::string("Font file not found: ") + p.string());
      assert(0 && "Font file not found");
   }

   return font;
}
