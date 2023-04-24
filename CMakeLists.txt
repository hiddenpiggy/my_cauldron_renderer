cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)


project(my_cauldron_renderer)
find_package(Vulkan REQUIRED)
add_subdirectory(libs/Vulkan-Hpp)
add_subdirectory(libs/Vulkan-Hpp/glm)
add_subdirectory(libs/Vulkan-Hpp/glfw)
add_subdirectory(libs/VulkanMemoryAllocator)


file(GLOB RENDERER_SRC_FILES src/*.cpp)

include_directories(
  ${Vulkan_INCLUDE_DIRS}
  libs/Vulkan-Hpp/Vulkan-Headers/include
  libs/Vulkan-Hpp/Vulkan-Headers/glm
  libs/Vulkan-Hpp
  libs/Vulkan-Hpp/glfw/include
  libs/VulkanMemoryAllocator/include
  include/
)

add_executable(Renderer ${RENDERER_SRC_FILES})
target_link_libraries(Renderer Vulkan::Vulkan glfw)


# shader compilation utils
# Find glslc in PATH
find_program(GLSLC_EXECUTABLE NAMES glslc)
if(NOT GLSLC_EXECUTABLE)
  message(FATAL_ERROR "glslc not found in PATH. Please install the Vulkan SDK or set the path to glslc manually.")
endif()
# Set glslc path to GLSLC_EXECUTABLE
set(GLSLC_EXECUTABLE ${GLSLC_EXECUTABLE} CACHE STRING "Path to glslc compiler")
# Define input and output directories
set(INPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)
set(OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)
# Get list of shader files in input directory
file(GLOB_RECURSE SHADER_FILES ${INPUT_DIR}/*.vert ${INPUT_DIR}/*.frag)
# Loop over shader files and add custom command for each shader
foreach(SHADER_FILE ${SHADER_FILES})
  # Get shader name without file extension
  get_filename_component(SHADER_NAME ${SHADER_FILE} NAME_WE)

  # Construct the command to compile shader
  set(SHADER_INPUT ${SHADER_FILE})
  set(SHADER_OUTPUT ${OUTPUT_DIR}/${SHADER_NAME}.spv)
  set(SHADER_COMMAND ${GLSLC_EXECUTABLE} -o ${SHADER_OUTPUT} ${SHADER_INPUT})

  # Add custom command for shader compilation
  add_custom_command(
    TARGET Renderer
    COMMAND ${SHADER_COMMAND}
    COMMENT "Compiling shader: ${SHADER_NAME}"
  )
endforeach()

set(SHADERS_PATH ${CMAKE_CURRENT_LIST_DIR}/shaders)
set(TEXTURES_PATH ${CMAKE_CURRENT_LIST_DIR}/textures)
set(SCENES_PATH ${CMAKE_CURRENT_LIST_DIR}/scenes)
target_compile_definitions(Renderer PRIVATE SHADERS_PATH="${SHADERS_PATH}/" TEXTURES_PATH="${TEXTURES_PATH}/" SCENES_PATH="${SCENES_PATH}/")


