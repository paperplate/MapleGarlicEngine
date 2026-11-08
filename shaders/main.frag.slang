import helpers;

struct Uniforms
{
   float2 uResolution;
   float uTime;
   int uFacing;
};

[[vk::binding(0, 3)]]
ParameterBlock<Uniforms> uniforms : register(b0, space3);

struct VertexOutput
{
   float4 position : SV_Position;
   float2 uv : TEXCOORD;
};

struct FragmentOutput
{
   float4 color : SV_Target;
};

// ==========================================
// 1. Central Configuration Header
// ==========================================
namespace Config
{
static const float Zoom = 1.75;
static const float OutlineThickness = 0.015;
static const float NoOutline = 0.0;

enum PartID : uint8_t
{
   EarR = 0,
   ArmR,
   FootR,
   Body,
   EarL,
   FootL,
   ArmL,
   Trunk,
   EyeL,
   EyeR,
   NUM_PARTS
};

namespace Colors
{
static const float4 SkyBlue = float4(0.53, 0.81, 0.92, 1.0);
static const float4 Pink = float4(0.98, 0.61, 0.76, 1.0);
static const float4 Red = float4(0.85, 0.2, 0.3, 1.0);
static const float4 Black = float4(0.1, 0.1, 0.1, 1.0);
}
}

// ==========================================
// 2. Encapsulated State
// ==========================================
struct Canvas
{
   float4 color;
   float2 uv;
   float2 res;

   /**
    * @brief Renders an anti-aliased elliptical shape onto the canvas using SDF blending.
    *
    * @param center The 2D coordinates representing the center of the shape.
    * @param radius The extents (half-width, half-height) of the ellipse.
    * @param angle The rotation angle of the shape in radians.
    * @param fillColor The base fill color of the shape (RGBA).
    * @param customOutline The mandatory outline thickness (use 0.0 for no outline).
    */
   [mutating]
   void ellipse(float2 center, float2 radius, float angle, float4 fillColor, float customOutline)
   {
      color = drawShape(color, uv, center, radius, angle, fillColor, customOutline, res);
   }

   [mutating]
   void ellipse(float2 center, float2 radius, float angle, float4 fillColor)
   {
      color = drawShape(color, uv, center, radius, angle, fillColor, Config::OutlineThickness, res);
   }
};

struct CharState
{
   float2 offsets[(int)Config::PartID::NUM_PARTS];
   float rotations[(int)Config::PartID::NUM_PARTS];
   float2 sizes[(int)Config::PartID::NUM_PARTS];
   float4 colors[(int)Config::PartID::NUM_PARTS];
   float outlines[(int)Config::PartID::NUM_PARTS];
   float2 pivotOffsets[(int)Config::PartID::NUM_PARTS];
   float2 baseSizes[(int)Config::PartID::NUM_PARTS];
   Config::PartID zOrder[(int)Config::PartID::NUM_PARTS];

   [mutating]
   void setIdentitiy(Config::PartID id, float2 baseSize, float4 color, float outline)
   {
      baseSizes[id] = baseSize;
      colors[id] = color;
      outlines[id] = outline;
      pivotOffsets[id] = float2(0.0, 0.0);
   }

   [mutating]
   void setPivot(Config::PartID id, float2 pivot)
   {
      pivotOffsets[id] = pivot;
   }

   [mutating]
   void place(Config::PartID id, float2 offset, float rot)
   {
      offsets[id] = offset;
      sizes[id] = baseSizes[id];
      rotations[id] = rot;
   }

   [mutating]
   void place(Config::PartID id, float2 offset, float2 customSize, float rot)
   {
      offsets[id] = offset;
      sizes[id] = customSize;
      rotations[id] = rot;
   }
};

struct KirbyAnim
{
   struct Sizes
   {
      static const float2 Body = float2(0.35, 0.35);
      static const float2 Ear = float2(0.2, 0.3);
      static const float2 Foot = float2(0.15, 0.08);
      static const float2 Arm = float2(0.12, 0.08);
      static const float2 Trunk = float2(0.06, 0.25);
      static const float2 Eye = float2(0.03, 0.08);
      static const float2 None = float2(0.0, 0.0);
   };

   struct Anim
   {
      static const float ArmSpeed = 5.0;
      static const float ArmAmp = 0.4;
      static const float TrunkSpeed = 3.0;
      static const float TrunkAmp = 0.2;
      static const float EarSpeed = 6.0;
      static const float EarAmp = 0.1;
   };

   float leftArmSwing;
   float rightArmSwing;
   float trunkSwing;
   float earFlap;

   float4 bodyColor = Config::Colors::SkyBlue;
   float4 eyeColor = Config::Colors::Black;

   CharState state = {};

   __init(float time)
   {
      leftArmSwing = -0.5 + sin(time * Anim::ArmSpeed) * Anim::ArmAmp;
      rightArmSwing = 0.5 - sin(time * Anim::ArmSpeed) * Anim::ArmAmp;
      trunkSwing = sin(time * Anim::TrunkSpeed) * Anim::TrunkAmp;
      earFlap = sin(time * Anim::EarSpeed) * Anim::EarAmp;
   }

   [mutating]
   void mirror()
   {
      float temp = leftArmSwing;
      leftArmSwing = -rightArmSwing;
      rightArmSwing = -temp;
   }

   [mutating]
   void setupIdentities()
   {
      using Config;
      state.setIdentitiy(PartID::Body, Sizes::Body, bodyColor, OutlineThickness);
      state.setIdentitiy(PartID::EarL, Sizes::Ear, bodyColor, OutlineThickness);
      state.setIdentitiy(PartID::EarR, Sizes::Ear, bodyColor, OutlineThickness);
      state.setIdentitiy(PartID::ArmL, Sizes::Arm, bodyColor, OutlineThickness);
      state.setIdentitiy(PartID::ArmR, Sizes::Arm, bodyColor, OutlineThickness);
      state.setIdentitiy(PartID::Trunk, Sizes::Trunk, bodyColor, OutlineThickness);
      state.setIdentitiy(PartID::FootL, Sizes::Foot, bodyColor, OutlineThickness);
      state.setIdentitiy(PartID::FootR, Sizes::Foot, bodyColor, OutlineThickness);
      state.setIdentitiy(PartID::EyeL, Sizes::Eye, eyeColor, NoOutline);
      state.setIdentitiy(PartID::EyeR, Sizes::Eye, eyeColor, NoOutline);

      state.setPivot(PartID::Trunk, float2(0.0, Sizes::Trunk.y));
   }

   [mutating]
   void buildLayout(int renderDir)
   {
      setupIdentities();

      using Config;

      for (int i = 0; i < (int)PartID::NUM_PARTS; ++i)
      {
         state.sizes[i] = Sizes::None;
      }

      if (renderDir < 0 || renderDir > 4)
      {
         return;
      }

      PartID order[(int)PartID::NUM_PARTS];
      if (renderDir == 0) // FRONT VIEW
      {
         state.place(PartID::Body, float2(0.0, 0.0), 0.0);
         state.place(PartID::EarL, float2(-0.35, -0.1), -0.2 - earFlap);
         state.place(PartID::EarR, float2(0.35, -0.1), 0.2 + earFlap);
         state.place(PartID::ArmL, float2(-0.35, 0.05), leftArmSwing);
         state.place(PartID::ArmR, float2(0.35, 0.05), rightArmSwing);
         state.place(PartID::FootL, float2(-0.15, 0.3), -0.1);
         state.place(PartID::FootR, float2(0.15, 0.3), 0.1);
         state.place(PartID::Trunk, float2(0.0, 0.15), trunkSwing);
         state.place(PartID::EyeL, float2(-0.08, -0.12), 0.0);
         state.place(PartID::EyeR, float2(0.08, -0.12), 0.0);

         order = { PartID::EarL, PartID::EarR, PartID::Body,  PartID::FootL, PartID::FootR,
                   PartID::ArmL, PartID::ArmR, PartID::Trunk, PartID::EyeL,  PartID::EyeR };
      }
      else if (renderDir == 1) // FRONT RIGHT VIEW
      {
         state.place(PartID::Body, float2(0.0, 0.0), 0.0);
         state.place(PartID::EarL, float2(-0.25, -0.1), float2(0.18, 0.28), -0.2 - earFlap);
         state.place(PartID::EarR, float2(0.3, -0.1), float2(0.1, 0.25), 0.2);
         state.place(PartID::ArmL, float2(-0.05, 0.08), leftArmSwing);
         state.place(PartID::ArmR, float2(0.35, 0.05), rightArmSwing);
         state.place(PartID::FootL, float2(-0.05, 0.3), -0.1);
         state.place(PartID::FootR, float2(0.2, 0.25), float2(0.12, 0.06), 0.1);
         state.place(PartID::Trunk, float2(0.28, 0.12), trunkSwing - 0.2);
         state.place(PartID::EyeL, float2(0.12, -0.12), 0.0);
         state.place(PartID::EyeR, float2(0.32, -0.12), float2(0.015, 0.08), 0.0);

         order = { PartID::EarR,  PartID::ArmR, PartID::FootR, PartID::Body, PartID::EarL,
                   PartID::FootL, PartID::ArmL, PartID::Trunk, PartID::EyeL, PartID::EyeR };
      }
      else if (renderDir == 2) // Profile (Right)
      {
         state.place(PartID::Body, float2(0.0, 0.0), 0.0);
         state.place(PartID::EarR, float2(-0.05, -0.1), float2(0.15, 0.3), 0.0);
         state.place(PartID::ArmR, float2(0.0, 0.1), rightArmSwing);
         state.place(PartID::FootR, float2(0.0, 0.3), 0.0);
         state.place(PartID::Trunk, float2(0.33, 0.12), trunkSwing - 0.4);
         state.place(PartID::EyeR, float2(0.25, -0.12), float2(0.015, 0.08), 0.0);

         order = {
            PartID::EarL, PartID::FootL, PartID::ArmL,  PartID::EyeL,  PartID::Body,
            PartID::EarR, PartID::ArmR,  PartID::FootR, PartID::Trunk, PartID::EyeR,
         };
      }
      else if (renderDir == 3) // Back-Right
      {
         state.place(PartID::Body, float2(0.0, 0.0), 0.0);
         state.place(PartID::EarL, float2(-0.15, -0.1), float2(0.12, 0.25), -0.1 - earFlap);
         state.place(PartID::EarR, float2(0.2, -0.1), float2(0.18, 0.28), 0.1 + earFlap);
         state.place(PartID::ArmL, float2(-0.25, 0.08), float2(0.08, 0.08), leftArmSwing);
         state.place(PartID::ArmR, float2(0.28, 0.05), float2(0.1, 0.08), rightArmSwing);
         state.place(PartID::FootL, float2(-0.15, 0.28), float2(0.12, 0.06), -0.1);
         state.place(PartID::FootR, float2(0.15, 0.25), float2(0.12, 0.06), 0.1);

         order = { PartID::ArmR,  PartID::FootR, PartID::EyeR, PartID::FootL, PartID::ArmL,
                   PartID::Trunk, PartID::EyeL,  PartID::Body, PartID::EarR,  PartID::EarL };
      }
      else if (renderDir == 4) // Back view
      {
         state.place(PartID::Body, float2(0.0, 0.0), 0.0);
         state.place(PartID::EarL, float2(-0.25, -0.1), float2(0.18, 0.3), -0.1 - earFlap);
         state.place(PartID::EarR, float2(0.25, -0.1), float2(0.18, 0.3), 0.1 + earFlap);
         state.place(PartID::ArmL, float2(-0.35, 0.05), leftArmSwing);
         state.place(PartID::ArmR, float2(0.35, 0.05), rightArmSwing);
         state.place(PartID::FootL, float2(-0.15, 0.3), -0.1);
         state.place(PartID::FootR, float2(0.15, 0.3), 0.1);

         order = { PartID::ArmL, PartID::ArmR, PartID::FootL, PartID::FootR, PartID::Trunk,
                   PartID::EyeL, PartID::EyeR, PartID::Body,  PartID::EarL,  PartID::EarR };
      }

      for (int i = 0; i < (int)PartID::NUM_PARTS; i++)
      {
         state.zOrder[i] = order[i];
      }
   }
};

void drawCharacter(inout Canvas c, KirbyAnim anim)
{
   for (int i = 0; i < (int)Config::PartID::NUM_PARTS; ++i)
   {
      int p = (int)anim.state.zOrder[i];
      if (anim.state.sizes[p].x > 0.0)
      {
         float2 center = anim.state.offsets[p];
         float2 pivotOffset = anim.state.pivotOffsets[p];
         float angle = anim.state.rotations[p];

         if (abs(pivotOffset.x) > 0.0 || abs(pivotOffset.y) > 0.0)
         {
            float2 pivot = center - pivotOffset;
            center = pivot + mul(pivotOffset, rotate(angle));
         }

         c.ellipse(center, anim.state.sizes[p], angle, anim.state.colors[p],
                   anim.state.outlines[p]);
      }
   }
}

[shader("fragment")]
FragmentOutput main(VertexOutput input)
{
   FragmentOutput output;

   float2 uv = input.uv - 0.5;
   uv.y = -uv.y;
   uv *= Config::Zoom;
   uv.y *= uniforms.uResolution.y / uniforms.uResolution.x;

   Canvas canvas = { Config::Colors::Pink, uv, uniforms.uResolution };
   KirbyAnim anim = KirbyAnim(uniforms.uTime);

   int dir = ((uniforms.uFacing % 8) + 8) % 8;
   int renderDir = dir;
   if (dir >= 5)
   {
      canvas.uv.x = -canvas.uv.x;
      anim.mirror();
      // renderDir = 8 - dir;
   }

   if (renderDir == 7)
   {
      renderDir = 1;
   }
   if (renderDir == 6)
   {
      renderDir = 2;
   }
   if (renderDir == 5)
   {
      renderDir = 3;
   }

   anim.buildLayout(renderDir);
   drawCharacter(canvas, anim);

   output.color = canvas.color;
   return output;
}

