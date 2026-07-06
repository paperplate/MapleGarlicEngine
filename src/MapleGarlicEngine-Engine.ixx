module;

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>

export module MapleGarlicEngine:Engine;
import :Texture;
import :Utils;

import std;

constexpr int gScreenWidth{640};
constexpr int gScreenHeight{480};

struct Vertex
{
   glm::vec3 position;
   glm::vec4 color;
};

struct UniformBuffer
{
   float time;
};

struct Particles
{
   struct Particle
   {
      float mSpeed;
      float mVelocity;
   };

   std::vector<SDL_FPoint> mPoints;
   std::vector<Particle> mParticles;
   Particles(size_t numOfPoints)
   {
      for (int i = 0; i < numOfPoints; i++)
      {
         Particle p{.mSpeed = static_cast<float>(SDL_rand(10)),
                    .mVelocity = static_cast<float>(SDL_rand(180))};
         SDL_FPoint point{.x = static_cast<float>(SDL_rand(gScreenWidth)),
                          .y = static_cast<float>(SDL_rand(10))};
         mPoints.push_back(point);
         mParticles.push_back(p);
      }
   }
};

struct Frame
{
   SDL_FRect mSrc;
   Frame(float x, float y, float w, float h) : mSrc{.x = x, .y = y, .w = w, .h = h}
   {
   }
};

struct Animation
{
   std::vector<Frame> mFrames;
   std::size_t mCurrentFrame = 0;
   std::size_t mMaxFrames = 0;

   void LoadFrames(float w, float h, int frameCount)
   {
      mMaxFrames = frameCount;
      for (int i = 0; i < frameCount; i++)
      {
         // 30, 160, 290, 410
         mFrames.push_back(Frame(i * 130 + 30, 40, 40, 40));
      }
   }

   SDL_FRect GetFrameAsSDL_FRect(int index) const
   {
      return mFrames.at(index).mSrc;
   }
   SDL_FRect GetFrameAsSDL_FRect() const
   {
      return mFrames.at(mCurrentFrame).mSrc;
   }

   void LoopAnimation()
   {
      mCurrentFrame++;
      if (mCurrentFrame >= mMaxFrames)
      {
         mCurrentFrame = 0;
      }
   }
};

struct Sprite
{
   Sprite(SDL_Renderer* r, std::string filename)
   {
      // SDL_Surface* surface = SDL_LoadBMP(filename.c_str());
      SDL_Surface* surface = IMG_Load(filename.c_str());
      const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(surface->format);
      const SDL_Palette* palette = SDL_GetSurfacePalette(surface);
      uint32_t colourKey = SDL_MapRGB(details, palette, 0xFF, 0x7F, 0x7F);
      SDL_SetSurfaceColorKey(surface, true, colourKey);
      mAnimation.LoadFrames(40, 40, 4);
      mTexture = SDL_CreateTextureFromSurface(r, surface);
      SDL_DestroySurface(surface);
   }

   ~Sprite()
   {
      if (mTexture)
      {
         SDL_DestroyTexture(mTexture);
         mTexture = nullptr;
      }
   }

   void SetPosition(float x, float y)
   {
      mDstRect.x = x;
      mDstRect.y = y;
   }

   void SetDimensions(float w, float h)
   {
      mDstRect.w = w;
      mDstRect.h = h;
   }

   void Update()
   {
      static int xDirection = 1;
      if (mDstRect.x > gScreenWidth - mDstRect.w)
      {
         xDirection = 0;
         mFlipMode = SDL_FLIP_HORIZONTAL;
      }
      if (mDstRect.x < 0)
      {
         xDirection = 1;
         mFlipMode = SDL_FLIP_NONE;
      }

      mDstRect.x += (xDirection == 1) ? 0.1f : -0.1f;

      mAnimation.LoopAnimation();
      mSrcRect = mAnimation.GetFrameAsSDL_FRect();
      // mDstRect.x = fmod(mDstRect.x, gScreenWidth);
   }

   void Render(SDL_Renderer* r)
   {
      SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xFF);
      SDL_RenderClear(r);

      SDL_SetRenderDrawColor(r, 0xFF, 0xFF, 0xFF, 0xFF);
      static int counter = 0;
      counter++;
      auto fmtStr = std::format("my str {}", counter);
      SDL_RenderDebugText(r, 100, 10, fmtStr.c_str());
      SDL_RenderDebugTextFormat(r, 200, 10, fmtStr.c_str());

      SDL_FPoint centre{.x = 0.0, .y = 0.0};
      SDL_RenderTextureRotated(r, mTexture, &mSrcRect, &mDstRect, 0.0, &centre, mFlipMode);
      // SDL_RenderTexture(r, mTexture, &mSrcRect, &mDstRect);

      SDL_SetTextureScaleMode(mTexture, SDL_SCALEMODE_LINEAR); // linear is default
   }

   SDL_FlipMode mFlipMode = SDL_FLIP_NONE;
   SDL_Texture* mTexture;
   SDL_FRect mSrcRect{.x = 30, .y = 40, .w = 40, .h = 40};
   SDL_FRect mDstRect{.x = 50, .y = 100, .w = 128, .h = 128};
   Animation mAnimation;
};

/*
auto windowDeleter = [](SDL_Window* w) { SDL_DestroyWindow(w); };
using SDL_WindowPtr = std::unique_ptr < SDL_Window, decltype(windowDeleter) > ;
SDL_WindowPtr mWindow{nullptr, windowDeleter };

mWindow.reset(SDL_CreateWindow(title, 320, 240, SDL_WINDOW_RESIZABLE));
assert(mWindow);
 */

export class Engine
{
 public:
   Engine()
      : mContinue{true}, mWindow{nullptr}, mRenderer{nullptr}, mDevice{nullptr}, mTimeUniform{},
        mPipeline{nullptr}, mVertexBuffer{nullptr}, mTexture{nullptr}, mSprite{nullptr}
   {
   }
   ~Engine()
   {
      mSprite.reset(nullptr);
      SDL_DestroyTexture(mFontText);
      TTF_Quit();
      if (mTexture)
      {
         // mTexture->Destroy();
         SDL_DestroyTexture(mTexture);
         mTexture = nullptr;
      }

      if (mRenderer)
      {
         SDL_DestroyRenderer(mRenderer);
         mRenderer = nullptr;
      }

      SDL_DestroyWindow(mWindow);
      mWindow = nullptr;

      SDL_Quit();
   }
   Engine(const Engine&) = delete;
   Engine& operator=(const Engine&) = delete;

   bool Init()
   {
      if (!SDL_Init(SDL_INIT_VIDEO))
      {
         log(LogType::ERROR, std::string("SDL_Init Error: ").append(SDL_GetError()));
         assert(0 && "SDL_Init(SDL_INIT_VIDEO) failed");
         return false;
      }

      if (!SDL_CreateWindowAndRenderer("MapleGarlicEngine", gScreenWidth, gScreenHeight,
                                       SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE, &mWindow,
                                       &mRenderer))
      {
         log(LogType::ERROR, std::string("SDL_CreateWindow Error: ").append(SDL_GetError()));
         return false;
      }

      SDL_SetRenderLogicalPresentation(mRenderer, gScreenWidth, gScreenHeight,
                                       SDL_LOGICAL_PRESENTATION_LETTERBOX);

      if (!TTF_Init())
      {
         log(LogType::ERROR, std::string("Unable to init TTF").append(SDL_GetError()));
         assert(0 && "TTF_INIT() failed");
         return false;
      }

      mFont = TTF_OpenFont("./UnboundedRegular-DYRl3.ttf", 16.0f);
      if (mFont == nullptr)
      {
         log(LogType::ERROR, "Failed to load font");
         return false;
      }

      SDL_Surface* textSurface =
         TTF_RenderText_Solid(mFont, "Hello", 0, SDL_Color{255, 255, 0, 255});
      mFontText = SDL_CreateTextureFromSurface(mRenderer, textSurface);
      SDL_DestroySurface(textSurface);

      SetupSceneData();

      /*SDL_FillSurfaceRect(mSurface, nullptr, 0x000000FF);

      mSurface = SDL_LoadBMP("./test.bmp");
      if (mSurface == nullptr)
      {
         std::cerr << "LoadBMP Error: " << SDL_GetError() << std::endl;
         return false;
      }*/

      SDL_ShowWindow(mWindow);

      // mTexture = std::make_unique<Texture>(&mWindow, &mRenderer);

      /*mDevice = {SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL |
          SDL_GPU_SHADERFORMAT_DXIL,
          true, // debug mode
          nullptr)};
      if (mDevice == nullptr)
      {
        std::cerr << "SDL_CreateGPUDevice Error: " << SDL_GetError() << std::endl;
        return false;
      }
      std::println("GPU backend: {}", SDL_GetGPUDeviceDriver(mDevice));

      if (!SDL_ClaimWindowForGPUDevice(mDevice, mWindow))
      {
        std::cerr << "SDL_ClaimWindowForGPUDevice Error: " << SDL_GetError() << std::endl;
        return false;
      }


      /*  SDL_GPUShader* vertexShader{LoadShader(mDevice, "shader.vert.spv", 0, 0, 0, 0)};
          if (!vertexShader)
          {
          log(LogType::ERROR, "Couldnt load vertex shader!");
          return false;
          }

          SDL_GPUShader* fragmentShader{LoadShader(mDevice, "shader.frag.spv", 0, 1, 0, 0)};
          if (!fragmentShader)
          {
          log(LogType::ERROR, "Couldnt load fragment shader!");
          return false;
          }

          std::vector<SDL_GPUColorTargetDescription> colorTargetDescriptions{
          {.format = SDL_GetGPUSwapchainTextureFormat(mDevice, mWindow)}};

          SDL_GPUGraphicsPipelineTargetInfo targetInfo{
          .color_target_descriptions = colorTargetDescriptions.data(),
          .num_color_targets = static_cast<uint32_t>(colorTargetDescriptions.size())};

          std::vector<SDL_GPUVertexAttribute> vertexAttributes{
          {.location = 0, // layout (location = 0) in shader
          .buffer_slot = 0,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
          .offset = 0},
          {.location = 1, // layout (location = 1) in shader
          .buffer_slot = 0,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
          .offset = sizeof(float) * 3} // 4th float form current buffer
          };

          std::vector<SDL_GPUVertexBufferDescription> vertexBufferDescriptions{};
          vertexBufferDescriptions.emplace_back(0, sizeof(Vertex), SDL_GPU_VERTEXINPUTRATE_VERTEX,
      0);

          SDL_GPUVertexInputState vertexInputState{
          .vertex_buffer_descriptions = vertexBufferDescriptions.data(),
          .num_vertex_buffers = static_cast<uint32_t>(vertexBufferDescriptions.size()),
          .vertex_attributes = vertexAttributes.data(),
          .num_vertex_attributes = static_cast<uint32_t>(vertexAttributes.size()),
          };

          SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo{
          .vertex_shader = vertexShader,
          .fragment_shader = fragmentShader,
          .vertex_input_state = vertexInputState,
          .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
          .rasterizer_state{.fill_mode = SDL_GPU_FILLMODE_FILL},
          .multisample_state = {},
          .depth_stencil_state = {},
          .target_info = targetInfo};

          mPipeline = {SDL_CreateGPUGraphicsPipeline(mDevice, &pipelineCreateInfo)};
          if (!mPipeline)
          {
          log(LogType::ERROR, "Failed to create GPU graphics pipeline");
          return false;
          }

          SDL_ReleaseGPUShader(mDevice, vertexShader);
          SDL_ReleaseGPUShader(mDevice, fragmentShader);

          std::vector<Vertex> vertices{{{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
          {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
          {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}};

          SDL_GPUBufferCreateInfo bufferCreateInfo{
          .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
          .size = static_cast<uint32_t>(vertices.size() * sizeof(Vertex)),
          .props = 0};
          mVertexBuffer = {SDL_CreateGPUBuffer(mDevice, &bufferCreateInfo)};
      if (!mVertexBuffer)
      {
        log(LogType::ERROR, "Failed to create vertex buffer");
        return false;
      }

      SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = bufferCreateInfo.size, .props = 0};
      auto* transferBuffer{SDL_CreateGPUTransferBuffer(mDevice, &transferBufferCreateInfo)};
      if (!transferBuffer)
      {
        log(LogType::ERROR, "Failed to create transfer buffer");
        return false;
      }

      auto transferBufferDataPtr{
        static_cast<Vertex*>(SDL_MapGPUTransferBuffer(mDevice, transferBuffer, false))};
      if (!transferBufferDataPtr)
      {
        log(LogType::ERROR, "Failed to map transfer buffer");
        return false;
      }

      std::span transferBufferData{transferBufferDataPtr, vertices.size()};

      std::ranges::copy(vertices, transferBufferData.begin());

      SDL_UnmapGPUTransferBuffer(mDevice, transferBuffer);

      SDL_GPUCommandBuffer* transferCommandBuffer{SDL_AcquireGPUCommandBuffer(mDevice)};
      if (!transferCommandBuffer)
      {
        log(LogType::ERROR, "Failed to acquire command buffer");
        return false;
      }

      SDL_GPUCopyPass* copyPass{SDL_BeginGPUCopyPass(transferCommandBuffer)};
      SDL_GPUTransferBufferLocation source{.transfer_buffer = transferBuffer, .offset = 0};
      SDL_GPUBufferRegion dest{.buffer = mVertexBuffer, .offset = 0, .size = bufferCreateInfo.size};
      SDL_UploadToGPUBuffer(copyPass, &source, &dest, true);
      SDL_EndGPUCopyPass(copyPass);

      if (!SDL_SubmitGPUCommandBuffer(transferCommandBuffer))
      {
        log(LogType::ERROR, "Failed to submit command buffer");
        return false;
      }

      SDL_ReleaseGPUTransferBuffer(mDevice, transferBuffer);*/

      return true;
   }

   bool Tick()
   {
      Input();
      Update();
      Render();
      /*SDL_GPUCommandBuffer* commandBuffer{SDL_AcquireGPUCommandBuffer(mDevice)};
        if (!commandBuffer)
        {
        return false;
        }

        SDL_GPUTexture* swapchainTexture{};
        uint32_t width;
        uint32_t height;
        SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, mWindow, &swapchainTexture, &width,
        &height);
        if (swapchainTexture)
        {
        mTimeUniform.time = SDL_GetTicksNS() / 1e9f; // time since program start in seconds
        SDL_PushGPUFragmentUniformData(commandBuffer, 0, &mTimeUniform, sizeof(UniformBuffer));

        SDL_GPUColorTargetInfo colorTarget{};
        colorTarget.texture = swapchainTexture;
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;
        colorTarget.clear_color = SDL_FColor{0.1f, 0.1f, 0.1f, 1.0f};
        colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;

        std::vector colorTargets{colorTarget};

        SDL_GPURenderPass* renderPass{SDL_BeginGPURenderPass(commandBuffer, colorTargets.data(),
        colorTargets.size(), nullptr)};
        SDL_BindGPUGraphicsPipeline(renderPass, mPipeline);

        std::vector<SDL_GPUBufferBinding> bindings{{.buffer = mVertexBuffer, .offset = 0}};
        SDL_BindGPUVertexBuffers(renderPass, 0, bindings.data(), bindings.size());
        SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
        SDL_EndGPURenderPass(renderPass);
        }

        if (!SDL_SubmitGPUCommandBuffer(commandBuffer))
        {
        std::cerr << "Couldnt submit GPU commandBuffer Error: " << SDL_GetError() << std::endl;
        return false;
        }

      /*SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
      SDL_RenderClear(mRenderer);

      mTexture->Render(0.0f, 0.0f);

      SDL_RenderPresent(mRenderer);*/

      return mContinue;
   }

   SDL_GPUShader* LoadShader(SDL_GPUDevice* device, const std::string& shaderFileName,
                             uint32_t samplerCount, uint32_t uniformBufferCount,
                             uint32_t storageBufferCount, uint32_t storageTextureCount)
   {
      SDL_GPUShaderStage stage;
      if (shaderFileName.contains(".vert"))
      {
         stage = SDL_GPU_SHADERSTAGE_VERTEX;
      }
      else if (shaderFileName.contains(".frag"))
      {
         stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
      }
      else
      {
         log(LogType::ERROR, "Invalid shader stage!");
         return nullptr;
      }

      const SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
      SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
      const char* entrypoint;

      std::string fullPath;
      const std::string BasePath{SDL_GetBasePath()};

      if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV)
      {
         fullPath = std::format("{}shaders/compiled/{}", BasePath, shaderFileName);
         format = SDL_GPU_SHADERFORMAT_SPIRV;
         entrypoint = "main";
      }
      // add metal, dx11

      std::ifstream file{fullPath, std::ios::binary};
      if (!file)
      {
         log(LogType::ERROR, std::string("Failed to load shader from disk! ").append(fullPath));
         return nullptr;
      }
      std::vector<uint8_t> code{std::istreambuf_iterator(file), {}};

      SDL_GPUShaderCreateInfo shaderInfo{
         .code_size = code.size(),
         .code = code.data(),
         .entrypoint = entrypoint,
         .format = format,
         .stage = stage,
         .num_samplers = samplerCount,
         .num_storage_textures = storageTextureCount,
         .num_storage_buffers = storageBufferCount,
         .num_uniform_buffers = uniformBufferCount,
         .props = 0 // no extensions needed yet
      };

      SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
      if (shader == nullptr)
      {
         log(LogType::ERROR, "Failed to create shader!");
         return nullptr;
      }

      return shader;
   }

   /*bool LoadMedia()
   {
      bool ret{true};

      if (!mTexture->LoadFromFile("logo.png"))
      {
         log(LogType::ERROR, "Unable to load PNG");
         ret = false;
      }

      return ret;
   }*/

   void SetupSceneData()
   {
      // mSprite = std::make_unique<Sprite>(mRenderer, "./test.bmp");
      mSprite = std::make_unique<Sprite>(mRenderer, "./_Attack.png");
   }

   SDL_Window* mWindow;
   SDL_Renderer* mRenderer;
   SDL_GPUDevice* mDevice;
   SDL_GPUGraphicsPipeline* mPipeline;
   SDL_GPUBuffer* mVertexBuffer;
   // std::unique_ptr<Texture> mTexture;
   SDL_Texture* mTexture;
   SDL_Surface* mSurface;
   std::unique_ptr<Sprite> mSprite;
   TTF_Font* mFont = nullptr;
   SDL_Texture* mFontText = nullptr;

 private:
   void Input()
   {
      SDL_Event event;
      while (SDL_PollEvent(&event))
      {
         if (event.type == SDL_EVENT_QUIT)
         {
            mContinue = false;
         }
         else if (event.type == SDL_EVENT_KEY_DOWN)
         {
            // user has pressed a key
            if (event.key.key == SDLK_ESCAPE)
            {
               mContinue = false;
            }
         }
      }
   }
   void ProcessEvents()
   {
   }
   void Update()
   {
      mSprite->Update();
      /*for (int i = 0; i < mParticles.mParticles.size(); i++)
      {
         mParticles.mPoints[i].y += mParticles.mParticles[i].mSpeed * 0.1f;
         mParticles.mPoints[i].x += SDL_sinf(mParticles.mParticles[i].mVelocity) * 2.0f;
         mParticles.mParticles[i].mVelocity += 0.1f;
         if (mParticles.mPoints[i].y > gScreenHeight)
         {
            mParticles.mPoints[i].y = 0;
         }
      }*/
   }
   void Render()
   {
      mSprite->Render(mRenderer);

      SDL_FRect textRect{.x = 20, .y = 120, .w = 100, .h = 20};
      SDL_RenderTexture(mRenderer, mFontText, nullptr, &textRect);
      SDL_RenderPresent(mRenderer);
      /*
       // colours can be modified by input
       SDL_SetRenderDrawColor(mRenderer, 0x00, 0x00, 0x00, 0xFF);
       SDL_RenderClear(mRenderer);

       SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
       // SDL_RenderLine(mRenderer, 0.0f, 0.0f, 100.0f, 50.0f);

       SDL_FRect rect{.x = 100, .y = 50, .w = 100, .h = 100};
       SDL_RenderRect(mRenderer, &rect);

       SDL_RenderPoints(mRenderer, mParticles.mPoints.data(), mParticles.mPoints.size());

       SDL_RenderPresent(mRenderer);
       /*SDL_Surface* windowSurface = SDL_GetWindowSurface(mWindow);
       if (windowSurface == nullptr)
       {
          log(LogType::ERROR, SDL_GetError());
          mContinue = false;
          return;
       }
       // nullptr == copy whole surface
       if (!SDL_BlitSurface(mSurface, nullptr, windowSurface, nullptr))
       {
          log(LogType::ERROR, "Blit surface error");
          return;
       }

       SDL_UpdateWindowSurface(mWindow);*/
   }

   Particles mParticles{1000};
   bool mContinue;
   UniformBuffer mTimeUniform;
};
