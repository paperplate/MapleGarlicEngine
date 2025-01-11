cmake_minimum_required(VERSION 3.31)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchain.cmake")
#set(TARGET 
project(VulkanTest VERSION 0.0.1 LANGUAGES CXX)

#set(CMAKE_CXX_MODULE_STD 1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_SCAN_FOR_MODULES ON)

#set(CMAKE_CXX_CLANG_TIDY ON)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts -fmodules-ts")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")# --enable-libstdcxx-backtrace")# -llibstdc++_libbacktrace")
add_compile_options(-fmodules-ts)
#add_compile_options(--enable-libstdcxx-backtrace=yes)
#add_compile_options(-lstdc++_libbacktrace)

#set(CMAKE_BUILD_TYPE debug)
set(CMAKE_BUILD_TYPE release)
#set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

find_package(Vulkan REQUIRED)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  include_directories(AFTER ${Vulkan_INCLUDE_DIRS}/Include)
  link_directories(${Vulkan_PATH}/Bin;${Vulkan_PATH}/Lib/)
endif()

# https://stackoverflow.com/questions/54834826/cannot-find-package-glfw-using-cmake
find_package(glfw3 REQUIRED)
set(GLFW_VULKAN_STATIC ON)

find_package(X11 REQUIRED)
find_package(Threads REQUIRED)

#find_program(GLSLC glslc)
find_program(GLSLC_EXECUTABLE NAMES glslc HINTS ENV PATH)
if (NOT GLSLC_EXECUTABLE)
	message(FATAL_ERROR "glslc not found.")
else()
	message(STATUS "using glslc: ${GLSLC_EXECUTABLE}")
endif()


set(SHADERS
  ${PROJECT_SOURCE_DIR}/shaders/shader.vert
  ${PROJECT_SOURCE_DIR}/shaders/shader.frag
)

function(compile_shader inputShader outputShader)
	add_custom_command(
		DEPENDS ${inputShader}
		OUTPUT ${outputShader}
		COMMAND ${GLSLC_EXECUTABLE} ${inputShader} -o ${outputShader}
		COMMENT "Compiling GLSL shader: ${inputShader}:"
		VERBATIM
	)
endfunction()

foreach(SHADER ${SHADERS})
	get_filename_component(SHADER_NAME ${SHADER} NAME)
	set(COMPILED_SHADER "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_NAME}.spv")
	compile_shader(${SHADER} ${COMPILED_SHADER})
	list(APPEND COMPILED_SHADERS ${COMPILED_SHADER})
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${COMPILED_SHADERS})

function(copy_files target directory)
	add_custom_command(
		TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/${directory}"
		"${CMAKE_CURRENT_BINARY_DIR}/${directory}"
	)
endfunction(copy_files)

add_custom_target(copy_textures ALL DEPENDS ${TEXTURES})
add_custom_target(copy_models ALL DEPENDS ${MODELS})

copy_files(copy_textures "textures")
copy_files(copy_models "models")

add_library(MapleGarlicEngine OBJECT 
)

target_sources(MapleGarlicEngine PUBLIC
	FILE_SET engine
	TYPE CXX_MODULES
	FILES helloTriangle.cxx
)

target_compile_features(MapleGarlicEngine
	PRIVATE cxx_std_23
	INTERFACE cxx_std_20
)

target_include_directories(MapleGarlicEngine PRIVATE
  ${GLFW_INCLUDE_DIRS}
  ${Vulkan_INCLUDE_DIR}
  ${X11_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
)

target_link_libraries(MapleGarlicEngine PRIVATE
	#${GLFW_LIB}
	glfw
	${Vulkan_LIBRARY}
  ${X11_LIBRARIES}
  ${X11_Xxf86vm_LIB}
  Threads::Threads
  ${CMAKE_DL_LIBS}
)


add_executable(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME}
	PRIVATE cxx_std_23
	INTERFACE cxx_std_20
)

target_sources(${PROJECT_NAME} PRIVATE
	main.cxx
	#tut.cpp
	)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${GLFW_INCLUDE_DIRS}
  ${Vulkan_INCLUDE_DIR}
  ${X11_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  MapleGarlicEngine
#	glfw
#	${Vulkan_LIBRARY}
#  ${X11_LIBRARIES}
#  ${X11_Xxf86vm_LIB}
#  Threads::Threads
#  ${CMAKE_DL_LIBS}
)
