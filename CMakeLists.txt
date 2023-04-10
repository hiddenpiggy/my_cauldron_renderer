cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)


project(my_cauldron_renderer)
find_package(Vulkan REQUIRED)
add_subdirectory(libs/Vulkan-Hpp)
add_subdirectory(libs/Vulkan-Hpp/glfw)

file(GLOB RENDERER_SRC_FILES src/*.cpp)

include_directories(
  ${Vulkan_INCLUDE_DIRS}
  libs/Vulkan-Hpp/Vulkan-Headers/include
  libs/Vulkan-Hpp
  libs/Vulkan-Hpp/glfw/include
  include/
)

add_executable(Renderer ${RENDERER_SRC_FILES})
target_link_libraries(Renderer Vulkan::Vulkan glfw)


