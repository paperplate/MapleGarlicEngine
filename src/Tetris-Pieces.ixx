module;
#include <SDL3/SDL.h>

export module Tetris:Pieces;
import MapleGarlicEngine.Utils;

import std;

export bool CheckCollision(const SDL_Rect& a, const SDL_Rect& b)
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

export std::vector<SDL_Rect> MakePiece(int x, int y, int w, int h)
{
   std::vector<SDL_Rect> out;
   for (size_t i = 0; i < 4; i++)
   {
      out.push_back(SDL_Rect{.x = x, .y = y, .w = w, .h = h});
   }
   return out;
}

export class LinePiece
{
 public:
   LinePiece(Color c)
      : X{0}, Y{0}, mColor{c}, mBoxWidth{10}, mBoxHeight{10}, mActive{false}, Gravity{3}
   {
      mRects = MakePiece(X, Y, mBoxWidth, mBoxHeight);
      mRects[0].x = X;
      mRects[0].y = Y;
      mRects[1].x = X;
      mRects[1].y = Y - mBoxHeight;
      mRects[2].x = X;
      mRects[2].y = Y + mBoxHeight;
      mRects[3].x = X;
      mRects[3].y = Y + (mBoxHeight * 2);
   }
   int GetGravity() const
   {
      return Gravity;
   }

   int X;
   int Y;
   const Color mColor;
   std::vector<SDL_Rect> mRects;

 private:
   const int mBoxWidth;
   const int mBoxHeight;
   bool mActive;
   int Gravity;
};

export class BoxPiece
{
 public:
   BoxPiece(Color c)
      : X{0}, Y{0}, mColor{c}, mBoxWidth{10}, mBoxHeight{10}, mActive{false}, Gravity{3}
   {
      mRects = MakePiece(X, Y, mBoxWidth, mBoxHeight);
      mRects[0].x = X;
      mRects[0].y = Y;
      mRects[1].x = X + mBoxWidth;
      mRects[1].y = Y;
      mRects[2].x = X;
      mRects[2].y = Y + mBoxHeight;
      mRects[3].x = X + mBoxWidth;
      mRects[3].y = Y + mBoxHeight;
   }
   int GetGravity() const
   {
      return Gravity;
   }

   int X;
   int Y;
   const Color mColor;
   std::vector<SDL_Rect> mRects;

 private:
   const int mBoxWidth;
   const int mBoxHeight;
   bool mActive;
   int Gravity;
};

export class TPiece
{
 public:
   TPiece(Color c)
      : X{0}, Y{0}, mColor{c}, mBoxWidth{10}, mBoxHeight{10}, mActive{false}, Gravity{3}
   {
      mRects = MakePiece(X, Y, mBoxWidth, mBoxHeight);
      mRects[0].x = X;
      mRects[0].y = Y;
      mRects[1].x = X - mBoxWidth;
      mRects[1].y = Y;
      mRects[2].x = X + mBoxWidth;
      mRects[2].y = Y;
      mRects[3].x = X;
      mRects[3].y = Y + mBoxHeight;
   }

   int GetGravity() const
   {
      return Gravity;
   }

   int X;
   int Y;
   const Color mColor;
   std::vector<SDL_Rect> mRects;

 private:
   const int mBoxWidth;
   const int mBoxHeight;
   bool mActive;
   int Gravity;
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
      for (auto& r : piece.mRects)
      {
         r.x = std::clamp(r.x + X, 0, gScreenWidth - r.w);
         r.y = std::clamp(r.y + Y + g, 0, gScreenHeight - r.h);
      }
   }
};

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
   for (size_t i = 0; i < n - 1; i++)
   {
      for (size_t j = 1; j < n; j++)
      {
         collisions[i] = CheckPieceCollision(pieces[i], pieces[j]);
      }
      if (!collisions[i])
      {
         std::visit(MovePiece{0, 0}, pieces[i]);
      }
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
