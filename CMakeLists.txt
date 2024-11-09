cmake_minimum_required(VERSION 3.22)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

project(VulkanTest VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE debug)

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

add_executable(${PROJECT_NAME})


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


target_sources(${PROJECT_NAME} PRIVATE
	helloTriangle.cpp
#tmp.cpp
	)	

target_include_directories(${PROJECT_NAME} PUBLIC
  ${GLFW_INCLUDE_DIRS}
  ${Vulkan_INCLUDE_DIR}
  ${X11_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	#${GLFW_LIB}
	glfw
	${Vulkan_LIBRARY}
  ${X11_LIBRARIES}
  ${X11_Xxf86vm_LIB}
  Threads::Threads
  ${CMAKE_DL_LIBS}
)
