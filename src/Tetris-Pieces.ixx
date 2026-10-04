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
   PieceBase(Color c)
      : mColor{c}, mX{0}, mY{0}, mBoxWidth{10}, mBoxHeight{10}, mActive{false}, Gravity{3}
   {
      mRects = MakePiece(mX, mY, mBoxWidth, mBoxHeight);
   }

   int GetGravity() const
   {
      return Gravity;
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

   const Color mColor;
   std::vector<SDL_Rect> mRects;

 protected:
   int mX;
   int mY;

   const int mBoxWidth;
   const int mBoxHeight;
   bool mActive;
   int Gravity;

   virtual void UpdatePosition() = 0;
};

export class LinePiece : public PieceBase
{
 public:
   LinePiece(Color c) : PieceBase(c)
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
   BoxPiece(Color c) : PieceBase(c)
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
   TPiece(Color c) : PieceBase(c)
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

/*
 * @return difference in x and y to valid move, or nullopt if no collision
 */
std::optional<std::pair<int, int>> CalcAllowedMovement(const auto& piece, int dX, int dY,
                                                       const std::vector<SDL_Rect>& obstacles,
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

      for (const auto& obs : obstacles)
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
      const int g = piece.GetGravity();
      const int proposedX = piece.X() + X;
      const int proposedY = piece.Y() + Y + g;
      auto m = CalcAllowedMovement(piece, proposedX, proposedY, piece.mRects, gScreenWidth,
                                   gScreenHeight);
      if (!m.has_value())
      {
         piece.SetPosition(proposedX, proposedY);
      }
      else
      {
         auto [x, y] = m.value();
         piece.SetPosition(x, y);
      }
   }
};

TEST_CASE("Move_Piece")
{
   LinePiece l(gGreen);
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
   const auto n = pieces.size();
   std::vector<bool> collisions(n);
   for (size_t i = 0; i < n; i++)
   {
      /*for (size_t j = 1; j < n; j++)
      {
         collisions[i] = CheckPieceCollision(pieces[i], pieces[j]);
      }
      if (!collisions[i])
      {
         std::visit(MovePiece{0, 0}, pieces[i]);
      }*/
      std::visit(MovePiece{0, 0}, pieces[i]);
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
