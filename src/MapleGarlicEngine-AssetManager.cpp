module;
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cassert>

module MapleGarlicEngine;
import :AssetManager;
import MapleGarlicEngine.Utils;

import std;

// Source - https://stackoverflow.com/a/4119881
// Posted by Timmmm, modified by community. See post 'Timeline' for change history
// Retrieved 2026-07-07, License - CC BY-SA 4.0

bool ichar_equals(char a, char b)
{
   return std::tolower(static_cast<unsigned char>(a)) ==
          std::tolower(static_cast<unsigned char>(b));
}

bool CaseInsensitiveCompare(std::string_view lhs, std::string_view rhs)
{
   return std::ranges::equal(lhs, rhs, ichar_equals);
}

SDL_Surface* LoadImage(const std::string& filename)
{
   SDL_Surface* surface = nullptr;

   std::filesystem::path p(filename);
   if (std::filesystem::exists(p))
   {
      if (CaseInsensitiveCompare(p.extension().string(), ".bmp"))
      {
         surface = SDL_LoadBMP(p.c_str());
      }
      else if (CaseInsensitiveCompare(p.extension().string(), ".png"))
      {
         surface = IMG_Load(p.c_str());
      }
      else
      {
         log(LogType::ERROR, std::string("Image file type not supported: ") + p.string());
         assert(0 && "Image file type not supported");
      }
   }
   else
   {
      log(LogType::ERROR, std::string("Image file not found: ") + p.string());
      assert(0 && "Image file not found");
   }

   return surface;
}

TTF_Font* LoadFont(const std::string& filename, const float size)
{
   TTF_Font* font = nullptr;

   std::filesystem::path p(filename);
   if (std::filesystem::exists(p))
   {
      font = TTF_OpenFont(p.c_str(), size);
   }
   else
   {
      log(LogType::ERROR, std::string("Font file not found: ") + p.string());
      assert(0 && "Font file not found");
   }

   return font;
}

// TODO refactor
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
