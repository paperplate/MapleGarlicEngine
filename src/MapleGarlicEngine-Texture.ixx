module;

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <cstdio>
#include <glm/glm.hpp>

export module MapleGarlicEngine:Texture;
import :Utils;

import std;

export class Texture
{
 public:
   Texture(SDL_Window** window, SDL_Renderer** renderer)
      : mTexture{nullptr}, mRenderer{*renderer}, mWindow{*window}, mWidth{0}, mHeight{0}
   {
   }
   ~Texture()
   {
      Destroy();
   }

   bool LoadFromFile(std::string path)
   {
      Destroy();

      SDL_Surface* loadedSurface = IMG_Load(path.c_str());
      if (loadedSurface == nullptr)
      {
         log(LogType::ERROR, "Unable to load image: " + path);
      }
      else
      {
         mTexture = SDL_CreateTextureFromSurface(mRenderer, loadedSurface);
         if (mTexture == nullptr)
         {
            log(LogType::ERROR, "Unable to create texture from loaded pixels");
         }
         else
         {
            mHeight = loadedSurface->h;
            mWidth = loadedSurface->w;
         }

         SDL_DestroySurface(loadedSurface);
      }
      return mTexture != nullptr;
   }

   void Destroy()
   {
      SDL_DestroyTexture(mTexture);
      mTexture = nullptr;
      mWidth = 0;
      mHeight = 0;
   }

   void Render(float x, float y)
   {
      // set texture position
      SDL_FRect dstRect{x, y, static_cast<float>(mWidth), static_cast<float>(mHeight)};

      SDL_RenderTexture(mRenderer, mTexture, nullptr, &dstRect);
   }

   int GetWidth() const
   {
      return mWidth;
   }

   int GetHeight() const
   {
      return mHeight;
   }

   bool IsLoaded() const
   {
      return mTexture != nullptr;
   }

 private:
   SDL_Texture* mTexture;
   SDL_Renderer* mRenderer;
   SDL_Window* mWindow;
   int mWidth;
   int mHeight;
};
