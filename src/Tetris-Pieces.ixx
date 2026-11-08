module;
#include <SDL3/SDL.h>
#include <doctest.h>

export module Tetris:Pieces;
import MapleGarlicEngine.Utils;

import std;

export bool CheckCollision(const SDL_Rect& a, const SDL_Rect& b)
{
   const int aMinX{a.x};
   const int aMaxX{a.x + a.w};
   const int aMinY{a.y};
   const int aMaxY{a.y + a.h};

   const int bMinX{b.x};
   const int bMaxX{b.x + b.w};
   const int bMinY{b.y};
   const int bMaxY{b.y + b.h};

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

TEST_CASE("CheckCollision_FlushContact")
{
   SDL_Rect rectA = {.x = 0, .y = 0, .w = 10, .h = 10};
   SDL_Rect rectB = {.x = 10, .y = 0, .w = 10, .h = 10};

   CHECK_FALSE(CheckCollision(rectA, rectB));

   SDL_Rect rectC = {.x = 9, .y = 0, .w = 10, .h = 10};
   CHECK(CheckCollision(rectA, rectC));
}

TEST_CASE("CheckCollision_Separation")
{
   SDL_Rect rectA = {.x = 0, .y = 0, .w = 10, .h = 10};
   SDL_Rect rectB = {.x = 11, .y = 0, .w = 10, .h = 10};

   CHECK_FALSE(CheckCollision(rectA, rectB));
}

export std::vector<SDL_Rect> MakePiece(int x, int y, int w, int h)
{
   std::vector<SDL_Rect> out;
   for (size_t i = 0; i < 4; i++)
   {
      out.push_back(SDL_Rect{.x = x, .y = y, .w = w, .h = h});
   }
   return out;
}

class PieceBase
{
 public:
   PieceBase(Color c, int gravity = 3)
      : mColor{c}, mX{0}, mY{0}, mBoxWidth{10}, mBoxHeight{10}, mActive{true}, mGravity{gravity}
   {
      mRects = MakePiece(mX, mY, mBoxWidth, mBoxHeight);
   }

   int GetGravity() const
   {
      return mGravity;
   }

   void SetPosition(int x, int y)
   {
      mX = x;
      mY = y;
      UpdatePosition();
   }

   int X() const
   {
      return mX;
   }

   int Y() const
   {
      return mY;
   }

   void SetActive(bool active)
   {
      mActive = active;
   }

   bool isActive() const
   {
      return mActive;
   }

   const Color mColor;
   std::vector<SDL_Rect> mRects;

 protected:
   int mX;
   int mY;

   const int mBoxWidth;
   const int mBoxHeight;
   bool mActive;
   int mGravity;

   virtual void UpdatePosition() = 0;
};

export class LinePiece : public PieceBase
{
 public:
   LinePiece(Color c, int gravity = 3) : PieceBase(c, gravity)
   {
      UpdatePosition();
   }

 private:
   void UpdatePosition() override
   {
      mRects[0].x = mX;
      mRects[0].y = mY;
      mRects[1].x = mX;
      mRects[1].y = mY + mBoxHeight;
      mRects[2].x = mX;
      mRects[2].y = mY + (mBoxHeight * 2);
      mRects[3].x = mX;
      mRects[3].y = mY + (mBoxHeight * 3);
   }
};

export class BoxPiece : public PieceBase
{
 public:
   BoxPiece(Color c, int gravity = 3) : PieceBase(c, gravity)
   {
      UpdatePosition();
   }

 private:
   void UpdatePosition() override
   {
      mRects[0].x = mX;
      mRects[0].y = mY;
      mRects[1].x = mX + mBoxWidth;
      mRects[1].y = mY;
      mRects[2].x = mX;
      mRects[2].y = mY + mBoxHeight;
      mRects[3].x = mX + mBoxWidth;
      mRects[3].y = mY + mBoxHeight;
   }
};

export class TPiece : public PieceBase
{
 public:
   TPiece(Color c, int gravity = 3) : PieceBase(c, gravity)
   {
      UpdatePosition();
   }

 private:
   void UpdatePosition() override
   {
      mRects[0].x = mX;
      mRects[0].y = mY;
      mRects[1].x = mX + mBoxWidth;
      mRects[1].y = mY;
      mRects[2].x = mX + (mBoxWidth * 2);
      mRects[2].y = mY;
      mRects[3].x = mX + mBoxWidth;
      mRects[3].y = mY + mBoxHeight;
   }
};

export using Piece = std::variant<LinePiece, BoxPiece, TPiece>;

export bool CheckPieceCollision(Piece& a, Piece& b)
{
   bool collision = false;
   std::visit(
      [&collision](auto& a, auto& b)
      {
         for (const auto& aa : a.mRects)
         {
            for (const auto& bb : b.mRects)
            {
               collision |= CheckCollision(aa, bb);
            }
         }
      },
      a, b);
   return collision;
}

constexpr bool SpansOverlap(int aMin, int aMax, int bMin, int bMax)
{
   return aMax > bMin && aMin < bMax;
}

PieceBase& GetBase(Piece& piece)
{
   return std::visit([](auto& p) -> PieceBase& { return p; }, piece);
}

const PieceBase& GetBase(const Piece& piece)
{
   return std::visit([](auto& p) -> const PieceBase& { return p; }, piece);
}

/*
 * @return difference in x and y to valid move, or nullopt if no collision
 */
std::optional<std::pair<int, int>> CalcAllowedMovement(const PieceBase& piece, int dX, int dY,
                                                       const std::vector<Piece>& obstacles,
                                                       int screenWidth, int screenHeight)
{
   int allowedDx = dX;
   int allowedDy = dY;

   for (const auto& r : piece.mRects)
   {
      int sqX = dX + r.x;
      int sqY = dY + r.y;
      int sqW = r.w;
      int sqH = r.h;

      for (const auto& obsPiece : obstacles)
      {
         const PieceBase& obsBase = GetBase(obsPiece);

         for (const auto& obs : obsBase.mRects)
         {
            if (dX != 0.0f && SpansOverlap(sqY, sqY + sqH, obs.y, obs.y + obs.h))
            {
               if (dX > 0 && sqX < obs.x)
               {
                  allowedDx = std::clamp(obs.x - (sqX + sqW), 0, allowedDx);
               }
               else if (dX < 0 && sqX > obs.x)
               {
                  allowedDx = std::clamp((obs.x + obs.w) - sqX, allowedDx, 0);
               }
            }

            if (dY != 0 && SpansOverlap(sqX, sqX + sqW, obs.x, obs.x + obs.w))
            {
               if (dY > 0 && sqY < obs.y)
               {
                  allowedDy = std::clamp(obs.y - (sqY + sqH), 0, allowedDy);
               }
               else if (dY < 0 && sqY > obs.y)
               {
                  allowedDy = std::clamp((obs.y + obs.h) - sqY, allowedDy, 0);
               }
            }
         }
      }

      if (dX > 0)
      {
         allowedDx = std::clamp(screenWidth - (sqX + sqW), 0, allowedDx);
      }
      else if (dX < 0)
      {
         allowedDx = std::clamp(0 - sqX, allowedDx, 0);
      }

      if (dY > 0)
      {
         allowedDy = std::clamp(screenHeight - (sqY + sqH), 0, allowedDy);
      }
      else if (dY < 0)
      {
         allowedDy = std::clamp(0 - sqY, allowedDy, 0);
      }
   }

   if (allowedDx != dX || allowedDy != dY)
   {
      return std::make_pair(allowedDx, allowedDy);
   }

   return std::nullopt;
}

export struct MovePiece
{
   int X;
   int Y;
   MovePiece(int x, int y) : X{x}, Y{y}
   {
   }
   void operator()(auto& piece) const
   {
      if (piece.isActive())
      {
         const int deltaX = X;
         const int deltaY = Y;
         auto m = CalcAllowedMovement(piece, deltaX, deltaY, {}, gScreenWidth, gScreenHeight);
         if (!m.has_value())
         {
            piece.SetPosition(piece.X() + deltaX, piece.Y() + deltaY);
         }
         else
         {
            auto [x, y] = m.value();
            piece.SetPosition(piece.X() + x, piece.Y() + y);
            piece.SetActive(false);
         }
      }
   }
};

TEST_CASE("Move_Piece_Line")
{
   LinePiece l(gGreen, 3);
   CHECK(l.X() == 0);
   CHECK(l.Y() == 0);

   MovePiece{50, 50}(l);
   CHECK(l.X() == 50);
   CHECK(l.Y() == 50);
}

TEST_CASE("Move_Piece_TPiece")
{
   TPiece l(gGreen, 3);
   CHECK(l.X() == 0);
   CHECK(l.Y() == 0);

   MovePiece{50, 50}(l);
   CHECK(l.X() == 50);
   CHECK(l.Y() == 50);
}

TEST_CASE("Move_Piece_Box")
{
   BoxPiece l(gGreen, 3);
   CHECK(l.X() == 0);
   CHECK(l.Y() == 0);

   MovePiece{50, 50}(l);
   CHECK(l.X() == 50);
   CHECK(l.Y() == 50);
}

export enum class Rotation { CW, CCW };

export struct RotatePiece
{
   void operator()(LinePiece& p, Rotation r) const
   {
   }
   void operator()(TPiece& p, Rotation r) const
   {
   }

   void operator()(auto&&, Rotation) const
   {
      return;
   }
};

export void UpdatePieces(std::vector<Piece>& pieces)
{
   std::vector<SDL_Rect> rects;

   PieceBase& p = GetBase(pieces.back());
   auto m = CalcAllowedMovement(p, 0, p.GetGravity(), {pieces.begin(), pieces.end() - 1},
                                gScreenWidth, gScreenHeight);
   auto [x, y] = m.has_value() ? m.value() : std::make_pair(0, 3);
   std::visit(MovePiece{x, y}, pieces.back());
   if (m.has_value())
   {
      p.SetPosition(p.X() + m->first, p.Y() + m->second);
      p.SetActive(false);
   }
   else
   {
      p.SetPosition(p.X(), p.Y());
   }
}

export void RenderPieces(SDL_Renderer* r, std::vector<Piece>& pieces)
{
   for (const auto& piece : pieces)
   {
      std::visit(
         [&r](auto& p)
         {
            SDL_SetRenderDrawColor(r, p.mColor.r, p.mColor.g, p.mColor.b, p.mColor.a);
            for (const auto& rect : p.mRects)
            {
               SDL_FRect drawingRect{.x = static_cast<float>(rect.x),
                                     .y = static_cast<float>(rect.y),
                                     .w = static_cast<float>(rect.w),
                                     .h = static_cast<float>(rect.h)};
               SDL_SetRenderDrawColor(r, p.mColor.r, p.mColor.g, p.mColor.b, p.mColor.a);
               SDL_RenderFillRect(r, &drawingRect);
               SDL_SetRenderDrawColor(r, gWhite.r, gWhite.g, gWhite.b, gWhite.a);
               SDL_RenderRect(r, &drawingRect);
            }
         },
         piece);
   }
}
