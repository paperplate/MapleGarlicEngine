module;
#include <SDL3/SDL.h>

export module Tetris:State;
import :Pieces;

import std;

export struct Title
{
   static Title instance;
   inline static std::string mStr = "Tetris, press Enter to start.";

   auto operator()(SDL_KeyboardEvent* key);
   void Render(SDL_Renderer* r, DynamicText* t);
};

export struct Playing
{
   static Playing instance;
   inline static uint64_t mPoints{0};
   inline static std::string mScore = "Score: ";
   auto operator()(SDL_KeyboardEvent* key);
   void Render(SDL_Renderer* r, DynamicText* t);
};

export struct GameOver
{
   static GameOver instance;
   inline static std::string mStr = "Game Over, press Enter to go back to Title.";
   auto operator()(SDL_KeyboardEvent* key);
   void Render(SDL_Renderer* r, DynamicText* t);
};

export using State = std::variant<Title, Playing, GameOver>;

inline auto Title::operator()(SDL_KeyboardEvent* key)
{
   if (key->key == SDLK_RETURN)
   {
      return State{Playing{}};
   }
   return State{*this};
}

void Title::Render(SDL_Renderer* r, DynamicText* t)
{
   t->SetText(mStr);
   int w = 100;
   int h = 50;
   t->Render(r, gScreenWidth / 2 - w, gScreenHeight / 2 - h, w * 2, h * 2);
}

inline auto Playing::operator()(SDL_KeyboardEvent* key)
{
   if (key->key == SDLK_RETURN)
   {
      return State{GameOver{}};
   }
   return State{*this};
}

void Playing::Render(SDL_Renderer* r, DynamicText* t)
{
   t->SetText(mScore + std::to_string(mPoints));
   t->Render(r, 30, 30, 100, 30);
}

inline auto GameOver::operator()(SDL_KeyboardEvent* key)
{
   if (key->key == SDLK_RETURN)
   {
      return State{Title{}};
   }
   return State{*this};
}

void GameOver::Render(SDL_Renderer* r, DynamicText* t)
{
   t->SetText(mStr);
   int w = 100;
   int h = 50;
   t->Render(r, gScreenWidth / 2 - w, gScreenHeight / 2 - h, w * 2, h * 2);
}

export class Tetris
{
 public:
   Tetris();
   ~Tetris();

   void Update(uint64_t dt);
   void Render(SDL_Renderer* r);
   std::optional<bool> ProcessEvents(SDL_Event* event);

 private:
   bool ChangeState();

   State mState;

   std::unique_ptr<DynamicText> mText;
   std::vector<Piece> mPieces;
   uint64_t mDt;
};

Tetris::Tetris() : mState{Title()}, mDt{0}
{
   mText = std::make_unique<DynamicText>(gFont);
   auto p0 = LinePiece(gRed);
   MovePiece{50, 50}(p0);
   mPieces.push_back(p0);

   auto p1 = BoxPiece(gGreen);
   MovePiece{150, 50}(p1);
   mPieces.push_back(p1);

   auto p2 = TPiece(gBlue);
   MovePiece{250, 50}(p2);
   mPieces.push_back(p2);
}
Tetris::~Tetris()
{
}

bool Tetris::ChangeState()
{
   return true;
}

void Tetris::Update(uint64_t dt)
{
   // TODO Refactor when timer is allowed to pause / restart
   /*mBoxes.Update();
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
   mPlayer.SetVelocity(0, 0);*/
   if (std::holds_alternative<Playing>(mState))
   {
      UpdatePieces(mPieces);
   }
}

void Tetris::Render(SDL_Renderer* r)
{

   // mBoxes.Render(r);
   // mPlayer.Render(r);

   std::visit([this, &r](auto state) { state.Render(r, mText.get()); }, mState);
   if (std::holds_alternative<Playing>(mState))
   {
      RenderPieces(r, mPieces);
   }
}

std::optional<bool> Tetris::ProcessEvents(SDL_Event* event)
{
   if (event->type == SDL_EVENT_KEY_DOWN)
   {
      [[maybe_unused]] int x = 0;
      [[maybe_unused]] int y = 0;
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
      mState = std::visit([&event](auto state) -> State { return state(&(event->key)); }, mState);
   }
   return true;
}
