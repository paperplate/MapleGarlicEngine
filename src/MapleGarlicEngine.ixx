module;

#include <SDL3/SDL.h>
#include <cstdio>
#include <glm/glm.hpp>

export module MapleGarlicEngine;

import std;

enum class LogType { INFO, WARNING, ERROR };

static void log(LogType type, std::string_view message) {
  switch (type) {
  case LogType::ERROR:
    std::println(stderr, "{}: {}", "ERROR", message);
    break;
  case LogType::WARNING:
  case LogType::INFO:
    std::println("{}: {}", "TEST", message);
    break;
  default:
    std::unreachable();
  }
}

struct Vertex {
  glm::vec3 position;
  glm::vec4 color;
};

struct UniformBuffer {
  float time;
};

SDL_GPUShader *LoadShader(SDL_GPUDevice *device,
                          const std::string &shaderFileName,
                          uint32_t samplerCount, uint32_t uniformBufferCount,
                          uint32_t storageBufferCount,
                          uint32_t storageTextureCount) {
  SDL_GPUShaderStage stage;
  if (shaderFileName.contains(".vert")) {
    stage = SDL_GPU_SHADERSTAGE_VERTEX;
  } else if (shaderFileName.contains(".frag")) {
    stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
  } else {
    log(LogType::ERROR, "Invalid shader stage!");
    return nullptr;
  }

  const SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
  SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
  const char *entrypoint;

  std::string fullPath;
  const std::string BasePath{SDL_GetBasePath()};

  if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV) {
    fullPath = std::format("{}shaders/compiled/{}", BasePath, shaderFileName);
    format = SDL_GPU_SHADERFORMAT_SPIRV;
    entrypoint = "main";
  }
  // add metal, dx11

  std::ifstream file{fullPath, std::ios::binary};
  if (!file) {
    log(LogType::ERROR,
        std::string("Failed to load shader from disk! ").append(fullPath));
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

  SDL_GPUShader *shader = SDL_CreateGPUShader(device, &shaderInfo);
  if (!shader) {
    log(LogType::ERROR, "Failed to create shader!");
    return nullptr;
  }

  return shader;
}

export class Engine {
public:
  Engine()
      : mIsRunning(true), mWindow(nullptr), mDevice(nullptr), mTimeUniform({}),
        mPipeline(nullptr), mVertexBuffer(nullptr) {
    SDL_Init(SDL_INIT_VIDEO);
  }
  ~Engine();
  Engine(const Engine &) = delete;
  Engine &operator=(const Engine &) = delete;

  bool Init() {
    mWindow = {SDL_CreateWindow("MapleGarlicEngine", 640, 480,
                                SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE)};
    if (mWindow == nullptr) {
      log(LogType::ERROR,
          std::string("SDL_CreateWindow Error: ").append(SDL_GetError()));
      return false;
    }

    mDevice = {SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV |
                                       SDL_GPU_SHADERFORMAT_MSL |
                                       SDL_GPU_SHADERFORMAT_DXIL,
                                   true, // debug mode
                                   nullptr)};
    if (!mDevice) {
      std::cerr << "SDL_CreateGPUDevice Error: " << SDL_GetError() << std::endl;
      return false;
    }
    std::println("GPU backend: {}", SDL_GetGPUDeviceDriver(mDevice));

    if (!SDL_ClaimWindowForGPUDevice(mDevice, mWindow)) {
      std::cerr << "SDL_ClaimWindowForGPUDevice Error: " << SDL_GetError()
                << std::endl;
      return false;
    }

    SDL_GPUShader *vertexShader{
        LoadShader(mDevice, "shader.vert.spv", 0, 0, 0, 0)};
    if (!vertexShader) {
      log(LogType::ERROR, "Couldnt load vertex shader!");
      return false;
    }

    SDL_GPUShader *fragmentShader{
        LoadShader(mDevice, "shader.frag.spv", 0, 1, 0, 0)};
    if (!fragmentShader) {
      log(LogType::ERROR, "Couldnt load fragment shader!");
      return false;
    }

    std::vector<SDL_GPUColorTargetDescription> colorTargetDescriptions{
        {.format = SDL_GetGPUSwapchainTextureFormat(mDevice, mWindow)}};

    SDL_GPUGraphicsPipelineTargetInfo targetInfo{
        .color_target_descriptions = colorTargetDescriptions.data(),
        .num_color_targets =
            static_cast<uint32_t>(colorTargetDescriptions.size())};

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
    vertexBufferDescriptions.emplace_back(0, sizeof(Vertex),
                                          SDL_GPU_VERTEXINPUTRATE_VERTEX, 0);

    SDL_GPUVertexInputState vertexInputState{
        .vertex_buffer_descriptions = vertexBufferDescriptions.data(),
        .num_vertex_buffers =
            static_cast<uint32_t>(vertexBufferDescriptions.size()),
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
    if (!mPipeline) {
      log(LogType::ERROR, "Failed to create GPU graphics pipeline");
      return false;
    }

    SDL_ReleaseGPUShader(mDevice, vertexShader);
    SDL_ReleaseGPUShader(mDevice, fragmentShader);

    std::vector<Vertex> vertices{
        {{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}};

    SDL_GPUBufferCreateInfo bufferCreateInfo{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = static_cast<uint32_t>(vertices.size() * sizeof(Vertex)),
        .props = 0};
    mVertexBuffer = {SDL_CreateGPUBuffer(mDevice, &bufferCreateInfo)};
    if (!mVertexBuffer) {
      log(LogType::ERROR, "Failed to create vertex buffer");
      return false;
    }

    SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = bufferCreateInfo.size,
        .props = 0};
    auto *transferBuffer{
        SDL_CreateGPUTransferBuffer(mDevice, &transferBufferCreateInfo)};
    if (!transferBuffer) {
      log(LogType::ERROR, "Failed to create transfer buffer");
      return false;
    }

    auto transferBufferDataPtr{static_cast<Vertex *>(
        SDL_MapGPUTransferBuffer(mDevice, transferBuffer, false))};
    if (!transferBufferDataPtr) {
      log(LogType::ERROR, "Failed to map transfer buffer");
      return false;
    }

    std::span transferBufferData{transferBufferDataPtr, vertices.size()};

    std::ranges::copy(vertices, transferBufferData.begin());

    SDL_UnmapGPUTransferBuffer(mDevice, transferBuffer);

    SDL_GPUCommandBuffer *transferCommandBuffer{
        SDL_AcquireGPUCommandBuffer(mDevice)};
    if (!transferCommandBuffer) {
      log(LogType::ERROR, "Failed to acquire command buffer");
      return false;
    }

    SDL_GPUCopyPass *copyPass{SDL_BeginGPUCopyPass(transferCommandBuffer)};
    SDL_GPUTransferBufferLocation source{.transfer_buffer = transferBuffer,
                                         .offset = 0};
    SDL_GPUBufferRegion dest{
        .buffer = mVertexBuffer, .offset = 0, .size = bufferCreateInfo.size};
    SDL_UploadToGPUBuffer(copyPass, &source, &dest, true);
    SDL_EndGPUCopyPass(copyPass);

    if (!SDL_SubmitGPUCommandBuffer(transferCommandBuffer)) {
      log(LogType::ERROR, "Failed to submit command buffer");
      return false;
    }

    SDL_ReleaseGPUTransferBuffer(mDevice, transferBuffer);

    SDL_ShowWindow(mWindow);
    return true;
  }

  bool Tick() {
    SDL_GPUCommandBuffer *commandBuffer{SDL_AcquireGPUCommandBuffer(mDevice)};
    if (!commandBuffer) {
      return false;
    }

    SDL_GPUTexture *swapchainTexture{};
    uint32_t width;
    uint32_t height;
    SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, mWindow,
                                          &swapchainTexture, &width, &height);
    if (swapchainTexture) {
      mTimeUniform.time =
          SDL_GetTicksNS() / 1e9f; // time since program start in seconds
      SDL_PushGPUFragmentUniformData(commandBuffer, 0, &mTimeUniform,
                                     sizeof(UniformBuffer));

      SDL_GPUColorTargetInfo colorTarget{};
      colorTarget.texture = swapchainTexture;
      colorTarget.store_op = SDL_GPU_STOREOP_STORE;
      colorTarget.clear_color = SDL_FColor{0.1f, 0.1f, 0.1f, 1.0f};
      colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;

      std::vector colorTargets{colorTarget};

      SDL_GPURenderPass *renderPass{SDL_BeginGPURenderPass(
          commandBuffer, colorTargets.data(), colorTargets.size(), nullptr)};
      SDL_BindGPUGraphicsPipeline(renderPass, mPipeline);

      std::vector<SDL_GPUBufferBinding> bindings{
          {.buffer = mVertexBuffer, .offset = 0}};
      SDL_BindGPUVertexBuffers(renderPass, 0, bindings.data(), bindings.size());
      SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
      SDL_EndGPURenderPass(renderPass);
    }

    if (!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
      std::cerr << "Couldnt submit GPU commandBuffer Error: " << SDL_GetError()
                << std::endl;
      return false;
    }

    return true;
  }

  SDL_Window *mWindow;
  SDL_GPUDevice *mDevice;
  SDL_GPUGraphicsPipeline *mPipeline;
  SDL_GPUBuffer *mVertexBuffer;

private:
  void ProcessEvents();
  void Update();
  void Render();
  bool mIsRunning;
  UniformBuffer mTimeUniform;
};
