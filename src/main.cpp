#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

import MapleGarlicEngine;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  *appstate = new Engine();
  auto *engine = static_cast<Engine *>(*appstate);
  if (!engine->Init()) {
    return SDL_APP_FAILURE;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  auto *engine = static_cast<Engine *>(appstate);

  if (engine->Tick()) {
    return SDL_APP_CONTINUE;
  }

  return SDL_APP_SUCCESS; // initiates shutdown
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
    return SDL_APP_SUCCESS;
  }

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  auto *engine = static_cast<Engine *>(appstate);
  SDL_DestroyWindow(engine->mWindow);
}
