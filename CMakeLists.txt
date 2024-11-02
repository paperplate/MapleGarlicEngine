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

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
	helloTriangle.cpp
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
