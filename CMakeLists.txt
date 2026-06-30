cmake_minimum_required(VERSION 4.1)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
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

add_executable(${PROJECT_NAME}
  src/main.cpp
)

target_sources(${PROJECT_NAME}
  PRIVATE
  FILE_SET CXX_MODULES FILES
  src/MapleGarlicEngine.ixx
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_MODULE_STD ON
)

target_compile_options(${PROJECT_NAME} PRIVATE -fmodules-ts)

target_link_libraries(${PROJECT_NAME} PRIVATE 
  SDL3::SDL3
  glm::glm
)

