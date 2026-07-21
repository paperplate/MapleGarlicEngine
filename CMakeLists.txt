cmake_minimum_required(VERSION 4.1)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(MapleGarlicEngine LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	8d1fd52e5ab5590e2c81768ace50c72bae28f2ed #refs/tags/1.0.3
)

FetchContent_MakeAvailable(glm)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
message("SDL3 Found: ${SDL3_FOUND}")
find_package(SDL3_image REQUIRED CONFIG)
message("SDL3_Image Found: ${SDL3_image_FOUND}")
find_package(SDL3_ttf REQUIRED CONFIG)
message("SDL3_ttf Found: ${SDL3_ttf_FOUND}")

add_library(${PROJECT_NAME} SHARED)

target_sources(${PROJECT_NAME}
  PUBLIC
  FILE_SET CXX_MODULES FILES
  src/MapleGarlicEngine.ixx
  src/MapleGarlicEngine-AssetManager.ixx
  src/MapleGarlicEngine-Engine.ixx
  src/MapleGarlicEngine-Utils.ixx
  PRIVATE
  src/MapleGarlicEngine-AssetManager.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_MODULE_STD ON
)

target_compile_options(${PROJECT_NAME} PRIVATE -fmodules-ts -Wall)

target_link_libraries(${PROJECT_NAME} PRIVATE
  SDL3::SDL3
  SDL3_image::SDL3_image
  SDL3_ttf::SDL3_ttf
  glm::glm
)


add_executable(EngineTest
  src/main.cpp
)

target_link_libraries(EngineTest PRIVATE
  SDL3::SDL3
  ${PROJECT_NAME}
)

add_executable(Tetris
  src/tetris.cpp
)

target_compile_options(Tetris PRIVATE -fmodules-ts -Wall)

target_link_libraries(Tetris PRIVATE
  SDL3::SDL3
  ${PROJECT_NAME}
)
