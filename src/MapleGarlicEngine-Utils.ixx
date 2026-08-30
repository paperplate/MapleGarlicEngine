module;
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cstdio>

export module MapleGarlicEngine.Utils;

import std;

export constexpr int gScreenWidth{640};
export constexpr int gScreenHeight{480};
export constexpr int gScreenFps{60};
export TTF_Font* gFont = nullptr;

export enum class LogType : int { INFO = 0, WARNING, ERROR };
constexpr std::string LogStr[] = {"INFO", "WARNING", "ERROR"};

export void log(LogType type, std::string_view message)
{
   // recommended to use SDL_Log instead of iostream,
   // some platforms like Android have issues with it
   switch (type)
   {
   case LogType::ERROR:
      std::println(stderr, "{}: {}", "ERROR", message);
      break;
   case LogType::WARNING:
   case LogType::INFO:
      std::println("{}: {}", "TEST", message);
      break;
   default:
      std::unreachable();
   }
}

export struct Color
{
   uint8_t r;
   uint8_t g;
   uint8_t b;
   uint8_t a;

   bool operator==(const Color& other) const
   {
      bool result = true;
      result &= r == other.r;
      result &= g == other.g;
      result &= b == other.b;
      result &= a == other.a;
      return result;
   }

   SDL_Color GetSDLColor() const
   {
      return SDL_Color{.r = r, .g = g, .b = b, .a = a};
   }

   SDL_FColor GetSDLFColor() const
   {
      return SDL_FColor{.r = static_cast<float>(r) / 255.0f,
                        .g = static_cast<float>(g) / 255.0f,
                        .b = static_cast<float>(b) / 255.0f,
                        .a = static_cast<float>(a) / 255.0f};
   }
};

export constexpr Color gWhite{.r = 255, .g = 255, .b = 255, .a = 255};
export constexpr Color gBlack{.r = 0, .g = 0, .b = 0, .a = 255};
export constexpr Color gRed{.r = 255, .g = 0, .b = 0, .a = 255};
export constexpr Color gGreen{.r = 0, .g = 255, .b = 0, .a = 255};
export constexpr Color gBlue{.r = 0, .g = 0, .b = 255, .a = 255};
export constexpr Color gYellow{.r = 255, .g = 255, .b = 0, .a = 255};

export class Timer
{
 public:
   Timer();

   void Start();
   void Stop();
   void Pause();
   void UnPause();

   uint64_t GetTicksNS();

   bool IsStarted() const;
   bool IsPaused() const;

 private:
   // clock time when timer started
   uint64_t mStartTicks;

   // ticks stored when timer was paused
   uint64_t mPausedTicks;

   bool mPaused;
   bool mStarted;
};

Timer::Timer() : mStartTicks{0}, mPausedTicks{0}, mPaused{false}, mStarted{false}
{
}

void Timer::Start()
{
   mStarted = true;
   mPaused = false;

   mStartTicks = SDL_GetTicksNS();
   mPausedTicks = 0;
}

void Timer::Stop()
{
   mStarted = false;
   mPaused = false;

   mStartTicks = 0;
   mPausedTicks = 0;
}

void Timer::Pause()
{
   if (mStarted && !mPaused)
   {
      mPaused = true;
      mPausedTicks = SDL_GetTicksNS() - mStartTicks;
      mStartTicks = 0;
   }
}

void Timer::UnPause()
{
   if (mStarted && mPaused)
   {
      mPaused = false;
      mStartTicks = SDL_GetTicksNS() - mPausedTicks;
      mPausedTicks = 0;
   }
}

uint64_t Timer::GetTicksNS()
{
   uint64_t time{0};

   if (mStarted)
   {
      if (mPaused)
      {
         time = mPausedTicks;
      }
      else
      {
         time = SDL_GetTicksNS() - mStartTicks;
      }
   }

   return time;
}

export class DynamicText
{
 public:
   DynamicText(TTF_Font* font) : mFont{font}
   {
   }
   ~DynamicText()
   {
      SDL_DestroyTexture(mFontTextTexture);
      TTF_CloseFont(mFont);
   }

   void SetText(const std::string& text)
   {
      mText = text;
   }

   void Render(SDL_Renderer* r, float x, float y, float w, float h)
   {
      if (mText != mCachedText)
      {
         SDL_Surface* textSurface =
            TTF_RenderText_Solid(mFont, mText.c_str(), 0, gYellow.GetSDLColor());

         mFontTextTexture = SDL_CreateTextureFromSurface(r, textSurface);

         SDL_DestroySurface(textSurface);
         mCachedText = mText;
      }

      SDL_FRect textRect{.x = x, .y = y, .w = w, .h = h};
      SDL_RenderTexture(r, mFontTextTexture, nullptr, &textRect);
   }

   std::string mText;
   SDL_Texture* mFontTextTexture = nullptr;

 private:
   std::string mCachedText;
   TTF_Font* mFont;
};
