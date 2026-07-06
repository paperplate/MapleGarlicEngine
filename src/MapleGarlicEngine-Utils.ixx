module;

#include <cstdio>

export module MapleGarlicEngine:Utils;

import std;

export enum class LogType : int { INFO = 0, WARNING, ERROR };
constexpr std::string LogStr[] = {
  "INFO", "WARNING", "ERROR"
};

export void log(LogType type, std::string_view message)
{
   // recommended to use SDL_Log instead of iostream,
   // some platforms like Android have issues with it
   switch (type)
   {
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
