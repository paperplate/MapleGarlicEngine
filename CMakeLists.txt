cmake_minimum_required(VERSION 4.1)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(MapleGarlicEngine LANGUAGES CXX)

#option(ENABLE_DOCTESTS ON)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
#include(Doctest)

include(FetchContent)
  set(DOCTEST_VERSION v2.5.3)
  FetchContent_Declare(
    doctest
    URL https://raw.githubusercontent.com/doctest/doctest/${DOCTEST_VERSION}/doctest/doctest.h
    DOWNLOAD_NO_EXTRACT TRUE
  )
FetchContent_MakeAvailable(doctest)
set(DOCTEST_INCLUDE_DIR ${doctest_SOURCE_DIR})
include_directories(${DOCTEST_INCLUDE_DIR})
message("*** ${DOCTEST_INCLUDE_DIR} ***")
#find_package(doctest)

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

set(PUBLIC_FILES
  src/MapleGarlicEngine.ixx
  src/MapleGarlicEngine-AssetManager.ixx
  src/MapleGarlicEngine-Engine.ixx
  src/MapleGarlicEngine-Utils.ixx
  src/Tetris-Pieces.ixx
  src/Tetris-State.ixx
  src/Tetris.ixx
)

set(PRIVATE_FILES
  src/MapleGarlicEngine-AssetManager.cpp
)

target_sources(${PROJECT_NAME}
  PUBLIC
  FILE_SET CXX_MODULES FILES
  ${PUBLIC_FILES}
  PRIVATE
  ${PRIVATE_FILES}
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  CXX_MODULE_STD ON
  POSITION_INDEPENDENT_CODE ON
)

target_compile_options(${PROJECT_NAME} PRIVATE -fmodules-ts -Wall)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  SDL3::SDL3
  SDL3_image::SDL3_image
  SDL3_ttf::SDL3_ttf
  glm::glm
)

target_compile_definitions(${PROJECT_NAME} 
  PUBLIC
  DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
)

target_compile_definitions(${PROJECT_NAME} 
  PRIVATE
  DOCTEST_CONFIG_IMPLEMENT
)

add_executable(Tetris
  src/tetris.cpp
)

target_compile_options(Tetris PRIVATE -fmodules-ts -Wall)
target_compile_definitions(Tetris PRIVATE DOCTEST_CONFIG_DISABLE)

target_link_libraries(Tetris PRIVATE
  SDL3::SDL3
  ${PROJECT_NAME}
)

add_executable(EngineTest
  src/tetris.cpp
)

target_link_libraries(EngineTest PRIVATE
  SDL3::SDL3
  ${PROJECT_NAME}
)
