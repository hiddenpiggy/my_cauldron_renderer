# set cmake minimum version
cmake_minimum_required(VERSION 3.26)

# project name
project(Renderer)

# set cauldron to build with vulkan support
set_property(GLOBAL PROPERTY GFX_API "VK")

# dependencies
find_package(vulkan)
add_subdirectory(libs/Cauldron)

# include necessary header directories
include_directories(
    ${Vulkan_INCLUDE_DIRS}
    libs/Cauldron/src/common
    libs/Cauldron/src/common/base
    libs/Cauldron/src/common/FidelityFX
    libs/Cauldron/src/common/GLTF
    libs/Cauldron/src/common/Icon
    libs/Cauldron/src/common/Misc
    lib/Cauldron/src/VK
    libs/Cauldron/src/VK/base
    libs/Cauldron/src/VK/GLTF
    libs/Cauldron/src/VK/PostProc
    libs/Cauldron/src/VK/shaders
    libs/Cauldron/src/VK/widgets
    libs/Cauldron/libs/VulkanMemoryAllocator
)

# search for all source files in src folder
file(GLOB RASTERIZATION_RENDERER_SRCS src/*.cpp)


# add build target for Rasterization Renderer
add_executable(Rasterization_Renderer WIN32 ${RASTERIZATION_RENDERER_SRCS})
target_link_libraries(Rasterization_Renderer PRIVATE Cauldron_VK ImGUI)
target_include_directories(Rasterization_Renderer PRIVATE include/)