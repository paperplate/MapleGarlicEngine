
#include <SDL3/SDL.h>

#include <glm/glm.hpp>

#include <cstdio>

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
  glm::vec2 position;
  glm::vec3 color;
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

int main() {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *win{SDL_CreateWindow("MapleGarlicEngine", 640, 480,
                                   SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE)};
  if (win == nullptr) {
    log(LogType::ERROR,
        std::string("SDL_CreateWindow Error: ").append(SDL_GetError()));
    SDL_Quit();
    return 1;
  }

  SDL_GPUDevice *device{SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV |
                                                SDL_GPU_SHADERFORMAT_MSL |
                                                SDL_GPU_SHADERFORMAT_DXIL,
                                            true, // debug mode
                                            nullptr)};
  if (!device) {
    std::cerr << "SDL_CreateGPUDevice Error: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 1;
  }
  std::println("GPU backend: {}", SDL_GetGPUDeviceDriver(device));

  if (!SDL_ClaimWindowForGPUDevice(device, win)) {
    std::cerr << "SDL_ClaimWindowForGPUDevice Error: " << SDL_GetError()
              << std::endl;
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 1;
  }

  SDL_GPUShader *vertexShader{
      LoadShader(device, "shader.vert.spv", 0, 0, 0, 0)};
  if (!vertexShader) {
    log(LogType::ERROR, "Couldnt load vertex shader!");
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 1;
  }

  SDL_GPUShader *fragmentShader{
      LoadShader(device, "shader.frag.spv", 0, 0, 0, 0)};
  if (!fragmentShader) {
    log(LogType::ERROR, "Couldnt load fragment shader!");
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 1;
  }

  SDL_GPUColorTargetDescription colorTargetDescription{
      .format = SDL_GetGPUSwapchainTextureFormat(device, win)};
  std::vector colorTargetDescriptions{colorTargetDescription};

  SDL_GPUGraphicsPipelineTargetInfo targetInfo{
      .color_target_descriptions = colorTargetDescriptions.data(),
      .num_color_targets =
          static_cast<uint32_t>(colorTargetDescriptions.size())};

  std::vector<SDL_GPUVertexAttribute> vertexAttributes{};
  vertexAttributes.emplace_back(0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0);
  vertexAttributes.emplace_back(1, 0, SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,
                                sizeof(float) * 3);

  std::vector<SDL_GPUVertexBufferDescription> vertexBufferDescriptions{};
  vertexBufferDescriptions.emplace_back(0, sizeof(glm::vec3),
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

  SDL_GPUGraphicsPipeline *pipeline{
      SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo)};
  if (!pipeline) {
    log(LogType::ERROR, "Failed to create GPU graphics pipeline");
    return 1;
  }

  SDL_ReleaseGPUShader(device, vertexShader);
  SDL_ReleaseGPUShader(device, fragmentShader);

  std::vector<Vertex> vertices{{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                               {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                               {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

  SDL_GPUBufferCreateInfo bufferCreateInfo{
      .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
      .size = static_cast<uint32_t>(vertices.size() * sizeof(Vertex)),
      .props = 0};
  auto *vertexBuffer{SDL_CreateGPUBuffer(device, &bufferCreateInfo)};
  if (!vertexBuffer) {
    log(LogType::ERROR, "Failed to create vertex buffer");
    return 1;
  }

  SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{
      .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
      .size = bufferCreateInfo.size,
      .props = 0};
  auto *transferBuffer{
      SDL_CreateGPUTransferBuffer(device, &transferBufferCreateInfo)};
  if (!transferBuffer) {
    log(LogType::ERROR, "Failed to create transfer buffer");
    return 1;
  }

  auto transferBufferDataPtr{static_cast<Vertex *>(
      SDL_MapGPUTransferBuffer(device, transferBuffer, false))};
  if (!transferBufferDataPtr) {
    log(LogType::ERROR, "Failed to map transfer buffer");
    return 1;
  }

  std::span transferBufferData{transferBufferDataPtr, vertices.size()};

  std::ranges::copy(vertices, transferBufferData.begin());

  SDL_UnmapGPUTransferBuffer(device, transferBuffer);

  SDL_GPUCommandBuffer *transferCommandBuffer{
      SDL_AcquireGPUCommandBuffer(device)};
  if (!transferCommandBuffer) {
    log(LogType::ERROR, "Failed to acquire command buffer");
    return 1;
  }

  SDL_GPUCopyPass *copyPass{SDL_BeginGPUCopyPass(transferCommandBuffer)};
  SDL_GPUTransferBufferLocation source{.transfer_buffer = transferBuffer,
                                       .offset = 0};
  SDL_GPUBufferRegion dest{
      .buffer = vertexBuffer, .offset = 0, .size = bufferCreateInfo.size};
  SDL_UploadToGPUBuffer(copyPass, &source, &dest, false);
  SDL_EndGPUCopyPass(copyPass);

  if (!SDL_SubmitGPUCommandBuffer(transferCommandBuffer)) {
    log(LogType::ERROR, "Failed to submit command buffer");
    return 1;
  }

  SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

  SDL_ShowWindow(win);

  SDL_Event e;
  bool quit = false;

  while (!quit) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        quit = true;
      }
    }

    SDL_GPUCommandBuffer *commandBuffer{SDL_AcquireGPUCommandBuffer(device)};
    if (!commandBuffer) {
      SDL_DestroyWindow(win);
      SDL_Quit();
      return 1;
    }

    SDL_GPUTexture *swapchainTexture{};
    SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, win, &swapchainTexture,
                                          nullptr, nullptr);
    if (swapchainTexture) {
      SDL_GPUColorTargetInfo colorTarget{};
      colorTarget.texture = swapchainTexture;
      colorTarget.store_op = SDL_GPU_STOREOP_STORE;
      colorTarget.clear_color = SDL_FColor{0.1f, 0.1f, 0.1f, 1.0f};
      colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;

      std::vector colorTargets{colorTarget};

      SDL_GPURenderPass *renderPass{SDL_BeginGPURenderPass(
          commandBuffer, colorTargets.data(), colorTargets.size(), nullptr)};
      SDL_BindGPUGraphicsPipeline(renderPass, pipeline);

      std::vector<SDL_GPUBufferBinding> bindings{{vertexBuffer, 0}};
      SDL_BindGPUVertexBuffers(renderPass, 0, bindings.data(), bindings.size());
      SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
      SDL_EndGPURenderPass(renderPass);
    }

    if (!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
      std::cerr << "Couldnt submit GPU commandBuffer Error: " << SDL_GetError()
                << std::endl;
      SDL_DestroyWindow(win);
      SDL_Quit();
      return 1;
    }
  }

  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}
