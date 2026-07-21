module;

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
// #include <glm/glm.hpp>
#include <cassert>

export module MapleGarlicEngine:Engine;
import :AssetManager;
import :Utils;

import std;

constexpr int gScreenWidth{640};
constexpr int gScreenHeight{480};
constexpr int gScreenFps{60};

struct Color
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

constexpr Color gWhite{.r = 255, .g = 255, .b = 255, .a = 255};
constexpr Color gBlack{.r = 0, .g = 0, .b = 0, .a = 255};
constexpr Color gRed{.r = 255, .g = 0, .b = 0, .a = 255};
constexpr Color gGreen{.r = 0, .g = 255, .b = 0, .a = 255};
constexpr Color gBlue{.r = 0, .g = 0, .b = 255, .a = 255};
constexpr Color gYellow{.r = 255, .g = 255, .b = 0, .a = 255};

struct DynamicText
{
   DynamicText()
   {
      mFont = LoadFont("./UnboundedRegular-DYRl3.ttf", 12.0f);
      if (mFont == nullptr)
      {
         log(LogType::ERROR, "Failed to load font");
         assert(0 && "Failed to load font");
      }
   }
   ~DynamicText()
   {
      SDL_DestroyTexture(mFontTextTexture);
      TTF_CloseFont(mFont);
   }

   void Render(SDL_Renderer* r, std::string text, float x, float y, float w, float h)
   {
      if (text != mCachedText)
      {
         SDL_Surface* textSurface =
            TTF_RenderText_Solid(mFont, text.c_str(), 0, gYellow.GetSDLColor());

         mFontTextTexture = SDL_CreateTextureFromSurface(r, textSurface);

         SDL_DestroySurface(textSurface);
         mCachedText = text;
      }

      SDL_FRect textRect{.x = x, .y = y, .w = w, .h = h};
      SDL_RenderTexture(r, mFontTextTexture, nullptr, &textRect);
   }

   std::string mCachedText;
   TTF_Font* mFont = nullptr;
   SDL_Texture* mFontTextTexture = nullptr;
};

bool CheckCollision(const SDL_Rect& a, const SDL_Rect& b)
{
   int aMinX{a.x};
   int aMaxX{a.x + a.w};
   int aMinY{a.y};
   int aMaxY{a.y + a.h};

   int bMinX{b.x};
   int bMaxX{b.x + b.w};
   int bMinY{b.y};
   int bMaxY{b.y + b.h};

   if (aMinX >= bMaxX)
   {
      return false;
   }

   if (aMaxX <= bMinX)
   {
      return false;
   }

   if (aMinY >= bMaxY)
   {
      return false;
   }

   if (aMaxY <= bMinY)
   {
      return false;
   }

   return true;
}

class Timer
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

struct Boxes
{
 public:
   struct Attributes
   {
      int velocityX;
      int velocityY;
      Color color;
   };

   Boxes(size_t numOfBoxes, bool isPlayer = false) : mIsPlayer{isPlayer}
   {
      for (size_t i = 0; i < numOfBoxes; i++)
      {
         Color c = gWhite;
         int n = SDL_rand(4);
         if (n == 1)
         {
            c = gRed;
         }
         else if (n == 2)
         {
            c = gGreen;
         }
         else if (n == 0)
         {
            c = gBlue;
         }
         SDL_Rect tmp{
            .x = SDL_rand(gScreenWidth - 10), .y = SDL_rand(gScreenHeight - 10), .w = 10, .h = 10};

         mAttr.emplace_back(Attributes{.velocityX = 1, .velocityY = 1, .color = c});
         mRects.push_back(tmp);
      }
   }

   void Render(SDL_Renderer* r)
   {
      // SDL3 internally batches commands, can optimize later
      for (auto&& [attrs, rect] : std::views::zip(mAttr, mRects))
      {
         SDL_SetRenderDrawColor(r, attrs.color.r, attrs.color.g, attrs.color.b, attrs.color.a);
         SDL_FRect drawingRect{.x = static_cast<float>(rect.x),
                               .y = static_cast<float>(rect.y),
                               .w = static_cast<float>(rect.w),
                               .h = static_cast<float>(rect.h)};
         SDL_RenderRect(r, &drawingRect);
      }
   }

   void SetVelocity(int x, int y)
   {
      mAttr[0].velocityX = x;
      mAttr[0].velocityY = y;
   }

   SDL_Rect GetBox() const
   {
      return mRects[0];
   }

   Attributes GetAttributes() const
   {
      return mAttr[0];
   }

   void Update()
   {
      if (mIsPlayer)
      {
         int newX = mRects[0].x + mAttr[0].velocityX;
         int newY = mRects[0].y + mAttr[0].velocityY;
         mRects[0].x = std::clamp(newX, 0, gScreenWidth - mRects[0].w);
         mRects[0].y = std::clamp(newY, 0, gScreenHeight - mRects[0].h);
      }
      // for (auto&& [attrs, rect] : std::views::zip(mAttr, mRects))
      //{
      //  int newX = rect.x + SDL_rand(3) - 1;
      //  int newY = rect.y + SDL_rand(3) - 1;

      // rect.x = std::clamp(newX, 0, gScreenWidth - rect.w);
      // rect.y = std::clamp(newY, 0, gScreenHeight - rect.h);
      //}
   }

 private:
   const bool mIsPlayer;
   std::vector<Attributes> mAttr;
   std::vector<SDL_Rect> mRects;
};

class Tetris
{
 public:
   Tetris();
   ~Tetris();

   void Update();
   void Render(SDL_Renderer* r);
   std::optional<bool> ProcessEvents(SDL_Event* event);

 private:
   Boxes mBoxes{1};
   Boxes mPlayer{1, true};

   struct Title
   {
      const char* mStr = "Tetris";
   };
   struct Playing
   {
      uint64_t mPoints{0};
      std::string mScore = "Score: ";
   };
   struct GameOver
   {
      const char* mStr = "Game Over";
   };

   using State = std::variant<Title, Playing, GameOver>;

   State mState;
};

Tetris::Tetris()
{
}
Tetris::~Tetris()
{
}

void Tetris::Update()
{
   // mParticles.Update();
   mBoxes.Update();
   mPlayer.Update();
   bool collided = CheckCollision(mBoxes.GetBox(), mPlayer.GetBox());
   if (collided)
   {
      auto attr = mPlayer.GetAttributes();
      attr.velocityX = -attr.velocityX;
      attr.velocityY = -attr.velocityY;
      mPlayer.SetVelocity(attr.velocityX, attr.velocityY);
      mPlayer.Update();
   }
   mPlayer.SetVelocity(0, 0);
}

void Tetris::Render(SDL_Renderer* r)
{
   mBoxes.Render(r);
   mPlayer.Render(r);
}

std::optional<bool> Tetris::ProcessEvents(SDL_Event* event)
{
   if (event->type == SDL_EVENT_KEY_DOWN)
   {
      int x = 0;
      int y = 0;
      if (event->key.key == SDLK_W)
      {
         y = -1;
      }
      else if (event->key.key == SDLK_A)
      {
         x = -1;
      }
      else if (event->key.key == SDLK_S)
      {
         y = 1;
      }
      else if (event->key.key == SDLK_D)
      {
         x = 1;
      }
      mPlayer.SetVelocity(x, y);
   }
   return true;
}

export class Engine
{
 public:
   Engine();
   ~Engine();
   Engine(const Engine&) = delete;
   Engine& operator=(const Engine&) = delete;

   bool Init();

   std::optional<bool> Tick();

   std::optional<bool> ProcessEvents(SDL_Event* event);

   void SetupSceneData();

   SDL_Window* mWindow;
   SDL_Renderer* mRenderer;
   SDL_Surface* mSurface;
   std::unique_ptr<DynamicText> mText;

   Tetris mGame;

   bool mVSyncEnabled;
   bool mFpsCapEnabled;
   Timer mCapTimer;
   uint64_t mRenderTimeNS;
   std::stringstream mTimeText;

 private:
   void Update();
   void Render();
};

Engine::Engine()
   : mWindow{nullptr}, mRenderer{nullptr}, mVSyncEnabled{true}, mFpsCapEnabled{false},
     mRenderTimeNS{0}
{
}

Engine::~Engine()
{
   TTF_Quit();

   if (mRenderer)
   {
      SDL_DestroyRenderer(mRenderer);
      mRenderer = nullptr;
   }

   SDL_DestroyWindow(mWindow);
   mWindow = nullptr;

   SDL_Quit();
}

bool Engine::Init()
{
   if (!SDL_Init(SDL_INIT_VIDEO))
   {
      log(LogType::ERROR, std::string("SDL_Init Error: ").append(SDL_GetError()));
      assert(0 && "SDL_Init(SDL_INIT_VIDEO) failed");
      return false;
   }

   if (!SDL_CreateWindowAndRenderer("MapleGarlicEngine", gScreenWidth, gScreenHeight,
                                    SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE, &mWindow, &mRenderer))
   {
      log(LogType::ERROR, std::string("SDL_CreateWindow Error: ").append(SDL_GetError()));
      return false;
   }

   SDL_SetRenderLogicalPresentation(mRenderer, gScreenWidth, gScreenHeight,
                                    SDL_LOGICAL_PRESENTATION_LETTERBOX);

   if (!SDL_SetRenderVSync(mRenderer, 1))
   {
      log(LogType::ERROR,
          std::string("Could not enable VSync! SDL Error: ").append(SDL_GetError()));
      assert(0 && "Enable VSync failed");
      return false;
   }

   if (!TTF_Init())
   {
      log(LogType::ERROR, std::string("Unable to init TTF").append(SDL_GetError()));
      assert(0 && "TTF_INIT() failed");
      return false;
   }

   mText = std::make_unique<DynamicText>();

   SetupSceneData();

   SDL_ShowWindow(mWindow);

   mCapTimer.Start();

   return true;
}

std::optional<bool> Engine::Tick()
{
   Update();
   Render();

   return true;
}

std::optional<bool> Engine::ProcessEvents(SDL_Event* event)
{
   std::optional<bool> result = std::nullopt;
   if (event->type == SDL_EVENT_QUIT)
   {
      result = false;
   }
   else if (event->type == SDL_EVENT_KEY_DOWN)
   {
      // user has pressed a key
      if (event->key.key == SDLK_ESCAPE)
      {
         result = false;
      }
      else if (event->key.key == SDLK_RETURN)
      {
         mVSyncEnabled = !mVSyncEnabled;
         SDL_SetRenderVSync(mRenderer, (mVSyncEnabled) ? 1 : SDL_RENDERER_VSYNC_DISABLED);
         result = true;
      }
      else if (event->key.key == SDLK_SPACE)
      {
         mFpsCapEnabled = !mFpsCapEnabled;
         result = true;
      }
   }

   if (!result.has_value())
   {
      result = mGame.ProcessEvents(event);
   }
   return result;
}

void Engine::SetupSceneData()
{
}

void Engine::Update()
{
   mGame.Update();
}
void Engine::Render()
{
   SDL_SetRenderDrawColor(mRenderer, gBlack.r, gBlack.g, gBlack.b, gBlack.a);
   SDL_RenderClear(mRenderer);
   if (mRenderTimeNS != 0)
   {
      double fps{1000000000.0 / static_cast<double>(mRenderTimeNS)};
      mTimeText.str("");
      mTimeText << "Frames per second " << (mVSyncEnabled ? "(VSync) " : "")
                << (mFpsCapEnabled ? "(Cap) " : "") << fps;
      mText->Render(mRenderer, mTimeText.str(), 5.0f, 5.0f, 300.0f, 20.0f);
   }

   mGame.Render(mRenderer);

   SDL_RenderPresent(mRenderer);

   mRenderTimeNS = mCapTimer.GetTicksNS();

   // if time remaining in frame
   constexpr uint64_t nsPerFrame = 1000000000 / gScreenFps;
   if (mFpsCapEnabled && mRenderTimeNS < nsPerFrame)
   {
      // sleep remaining frame time
      SDL_DelayNS(nsPerFrame - mRenderTimeNS);

      // get frame time including sleep time
      mRenderTimeNS = mCapTimer.GetTicksNS();
   }
}
