module;

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
// #include <glm/glm.hpp>
#include <cassert>

export module MapleGarlicEngine:Engine;
import :AssetManager;
import MapleGarlicEngine.Utils;
import Tetris;

import std;

// constexpr int gScreenWidth{640};
// constexpr int gScreenHeight{480};
// constexpr int gScreenFps{60};
// TTF_Font* gFont = nullptr;

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

   std::unique_ptr<Tetris> mGame;

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

   gFont = LoadFont("./UnboundedRegular-DYRl3.ttf", 14.0f);
   if (!gFont)
   {
      log(LogType::ERROR, std::string("Unable to load font"));
      assert(0 && "LoadFont failed");
      return false;
   }

   mText = std::make_unique<DynamicText>(gFont);

   SetupSceneData();

   mGame = std::make_unique<Tetris>();

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
      else if (event->key.key == SDLK_P)
      {
         mVSyncEnabled = !mVSyncEnabled;
         SDL_SetRenderVSync(mRenderer, (mVSyncEnabled) ? 1 : SDL_RENDERER_VSYNC_DISABLED);
         result = true;
      }
      else if (event->key.key == SDLK_O)
      {
         mFpsCapEnabled = !mFpsCapEnabled;
         result = true;
      }
   }

   if (!result.has_value())
   {
      result = mGame->ProcessEvents(event);
   }
   return result;
}

void Engine::SetupSceneData()
{
}

void Engine::Update()
{
   mGame->Update(mCapTimer.GetTicksNS());
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
      mText->SetText(mTimeText.str());
      mText->Render(mRenderer, 5.0f, 5.0f, 300.0f, 20.0f);
   }

   mGame->Render(mRenderer);

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
