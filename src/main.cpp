#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

import MapleGarlicEngine;
import std;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
   *appstate = new Engine();
   auto* engine = static_cast<Engine*>(*appstate);
   if (!engine->Init())
   {
      return SDL_APP_FAILURE;
   }
   return SDL_APP_CONTINUE;
}

uint64_t fps = 0;
uint64_t lastTime = 0;
SDL_AppResult SDL_AppIterate(void* appstate)
{
   auto* engine = static_cast<Engine*>(appstate);

   uint64_t currentTick = SDL_GetTicks();
   auto run = engine->Tick();
   fps++;

   if (currentTick > lastTime + 1000)
   {
      // SDL_SetRenderDrawColor(engine->mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
      // SDL_RenderDebugText(engine->mRenderer, 10, 10, std::to_string(fps).c_str());
      std::println("{}", fps);
      lastTime = currentTick;
      fps = 0;
   }

   return (run) ? SDL_APP_CONTINUE : SDL_APP_SUCCESS; // initiates shutdown
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
   if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
   {
      return SDL_APP_SUCCESS;
   }
   else if (event->type == SDL_EVENT_KEY_DOWN)
   {
      // user has pressed a key
      if (event->key.key == SDLK_ESCAPE)
      {
         return SDL_APP_SUCCESS;
      }
   }

   return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
   auto* engine = static_cast<Engine*>(appstate);
   SDL_DestroyWindow(engine->mWindow);
}
