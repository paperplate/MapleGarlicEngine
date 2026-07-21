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

SDL_AppResult SDL_AppIterate(void* appstate)
{
   auto* engine = static_cast<Engine*>(appstate);

   auto run = engine->Tick();

   return (run) ? SDL_APP_CONTINUE : SDL_APP_SUCCESS; // initiates shutdown
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
   auto* engine = static_cast<Engine*>(appstate);

   auto result = engine->ProcessEvents(event);
   if (!result.has_value())
   {
      return SDL_APP_FAILURE;
   }

   return result.value() ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
   /*if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
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

   return SDL_APP_CONTINUE;*/
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
}
