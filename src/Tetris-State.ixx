module;
#include <SDL3/SDL.h>

export module Tetris:State;
import MapleGarlicEngine.Utils;
import :Pieces;

import std;

class Kirby
{
 public:
   ~Kirby()
   {
      if (mPipeline)
      {
         SDL_ReleaseGPUGraphicsPipeline(mDevice, mPipeline);
      }
   }

   static std::expected<std::unique_ptr<Kirby>, std::string> Create(SDL_GPUDevice* device,
                                                                    SDL_Window* window)
   {
      auto vertSpv = ReadBinaryFile("shaders/compiled/main.vert.spv");
      auto fragSpv = ReadBinaryFile("shaders/compiled/main.frag.spv");
      if (!vertSpv || !fragSpv)
      {
         return std::unexpected(vertSpv ? fragSpv.error() : vertSpv.error());
      }

      SDL_GPUShaderCreateInfo vertInfo{.code_size = vertSpv->size(),
                                       .code = reinterpret_cast<const uint8_t*>(vertSpv->data()),
                                       .entrypoint = "main",
                                       .format = SDL_GPU_SHADERFORMAT_SPIRV,
                                       .stage = SDL_GPU_SHADERSTAGE_VERTEX};
      SDL_GPUShader* vertShader = SDL_CreateGPUShader(device, &vertInfo);

      SDL_GPUShaderCreateInfo fragInfo{.code_size = fragSpv->size(),
                                       .code = reinterpret_cast<const uint8_t*>(fragSpv->data()),
                                       .entrypoint = "main",
                                       .format = SDL_GPU_SHADERFORMAT_SPIRV,
                                       .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
                                       .num_uniform_buffers = 1};
      SDL_GPUShader* fragShader = SDL_CreateGPUShader(device, &fragInfo);

      SDL_GPUColorTargetDescription colorTargetDesc{.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                                                    .blend_state = {.color_write_mask = 0xF}};

      SDL_GPUGraphicsPipelineCreateInfo pipeInfo{
         .vertex_shader = vertShader,
         .fragment_shader = fragShader,
         .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
         .target_info = {.color_target_descriptions = &colorTargetDesc, .num_color_targets = 1}};

      SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);

      SDL_ReleaseGPUShader(device, vertShader);
      SDL_ReleaseGPUShader(device, fragShader);

      if (!pipeline)
      {
         return std::unexpected(std::format("{} {}:{} Failed to create pipeline: {}", __FILE__,
                                            __FUNCTION__, __LINE__, SDL_GetError()));
      }

      return std::unique_ptr<Kirby>(new Kirby(device, pipeline));
   }

   void Render(SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* pass, int width, int height)
   {
      SDL_BindGPUGraphicsPipeline(pass, mPipeline);

      Uniforms uniforms = {.resX = static_cast<float>(width),
                           .resY = static_cast<float>(height),
                           .time = static_cast<float>(SDL_GetTicks() - mStartTime) / 1000.0f,
                           .facing = mFacing};

      SDL_PushGPUFragmentUniformData(cmd, 0, &uniforms, sizeof(Uniforms));
      SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
   }

   int mFacing = 0;

 private:
   Kirby(SDL_GPUDevice* device, SDL_GPUGraphicsPipeline* pipeline)
      : mDevice(device), mPipeline(pipeline)
   {
      mStartTime = SDL_GetTicks();
   }
   struct Uniforms
   {
      float resX;
      float resY;
      float time;
      int facing;
   };

   SDL_GPUDevice* mDevice;
   SDL_GPUGraphicsPipeline* mPipeline;
   uint64_t mStartTime;
};

export struct Title
{
   static Title instance;
   inline static SDL_GPUDevice* mDevice = nullptr;
   inline static SDL_Window* mWindow = nullptr;
   inline static std::unique_ptr<Kirby> mKirby;
   inline static SDL_Texture* mKirbyTarget = nullptr;
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
   else if (key->key == SDLK_A)
   {
      if (mKirby)
      {
         mKirby->mFacing--;
         return State{*this};
      }
   }
   else if (key->key == SDLK_D)
   {
      if (mKirby)
      {
         mKirby->mFacing++;
         return State{*this};
      }
   }
   return State{*this};
}

void Title::Render(SDL_Renderer* r, DynamicText* t)
{
   if (mKirby)
   {
      constexpr float renderScale = 0.4f;
      constexpr int targetWidth = gScreenWidth * renderScale;
      constexpr int targetHeight = gScreenHeight * renderScale;

      if (!mKirbyTarget)
      {
         mKirbyTarget = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET,
                                          targetWidth, targetHeight);
         SDL_SetTextureBlendMode(mKirbyTarget, SDL_BLENDMODE_BLEND);
      }

      SDL_FlushRenderer(r);

      SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(mDevice);

      SDL_PropertiesID texProps = SDL_GetTextureProperties(mKirbyTarget);
      SDL_GPUTexture* gpuTex = (SDL_GPUTexture*)(SDL_GetPointerProperty(
         texProps, SDL_PROP_TEXTURE_GPU_TEXTURE_POINTER, nullptr));

      if (gpuTex)
      {
         SDL_GPUColorTargetInfo colorTarget{.texture = gpuTex,
                                            .clear_color = {0.0f, 0.0f, 0.0f, 0.0f},
                                            .load_op = SDL_GPU_LOADOP_CLEAR,
                                            .store_op = SDL_GPU_STOREOP_STORE};
         SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &colorTarget, 1, nullptr);

         mKirby->Render(cmd, pass, gScreenWidth, gScreenHeight);

         SDL_EndGPURenderPass(pass);
      }

      SDL_SubmitGPUCommandBuffer(cmd);

      SDL_RenderTexture(r, mKirbyTarget, nullptr, nullptr);
   }

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

Piece createPiece(uint8_t n)
{
   if (n == 0)
   {
      return LinePiece(gRed);
   }
   else if (n == 1)
   {
      return BoxPiece(gGreen);
   }
   else
   {
      return TPiece(gBlue);
   }
}

export class Tetris
{
 public:
   Tetris(SDL_GPUDevice* device, SDL_Window* window) : mDevice(device), mWindow(window), mDt(0)
   {
      mText = std::make_unique<DynamicText>(gFont);
      Title title;
      title.mDevice = device;
      title.mWindow = window;
      auto kirby = Kirby::Create(device, window);
      if (kirby)
      {
         title.mKirby = std::move(*kirby);
      }
      else
      {
         log(LogType::ERROR, kirby.error());
      }
      mState = title;
   }
   ~Tetris()
   {
      Title::mKirby.reset();
      if (Title::mKirbyTarget)
      {
         SDL_DestroyTexture(Title::mKirbyTarget);
         Title::mKirbyTarget = nullptr;
      }
   }

   void Update(uint64_t dt)
   {
      if (mPieces.size() == 0)
      {
         mPieces.push_back(createPiece(dt % 3));
      }
      else
      {
         auto isActive =
            std::visit([](const auto& piece) -> bool { return piece.isActive(); }, mPieces.back());
         if (!isActive)
         {
            mPieces.push_back(createPiece(dt % 3));
         }
      }
      if (std::holds_alternative<Playing>(mState))
      {
         UpdatePieces(mPieces);
      }
   }

   void Render(SDL_Renderer* r)
   {
      std::visit([this, &r](auto state) { state.Render(r, mText.get()); }, mState);
      if (std::holds_alternative<Playing>(mState))
      {
         RenderPieces(r, mPieces);
      }
   }

   std::optional<bool> ProcessEvents(SDL_Event* event)
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
         mState =
            std::visit([&event](auto state) -> State { return state(&(event->key)); }, mState);
      }
      return true;
   }

 private:
   bool ChangeState()
   {
      return true;
   }

   State mState;

   SDL_GPUDevice* mDevice;
   SDL_Window* mWindow;

   std::unique_ptr<DynamicText> mText;
   std::vector<Piece> mPieces;
   uint64_t mDt;
};
