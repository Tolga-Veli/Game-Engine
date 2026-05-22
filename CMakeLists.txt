cmake_minimum_required(VERSION 3.20)
project(JupiterEngine VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LIB_DIR "${PROJECT_SOURCE_DIR}/libs")
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")

file(GLOB_RECURSE SRC_FILES
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.hpp"
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

# GLFW
add_subdirectory("${LIB_DIR}/glfw")
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# GLAD
add_library(glad "${LIB_DIR}/glad/src/gl.c")
target_include_directories(glad PUBLIC "${LIB_DIR}/glad/include")
target_link_libraries(${PROJECT_NAME} PRIVATE glad)

# STB_IMAGE
target_include_directories(${PROJECT_NAME} PRIVATE "${LIB_DIR}/stb")

# GLM
target_include_directories(${PROJECT_NAME} PRIVATE "${LIB_DIR}/glm")

# IMGUI
add_library(imgui STATIC
    "${LIB_DIR}/imgui/imgui.cpp"
    "${LIB_DIR}/imgui/imgui_draw.cpp"
    "${LIB_DIR}/imgui/imgui_tables.cpp"
    "${LIB_DIR}/imgui/imgui_widgets.cpp"
    "${LIB_DIR}/imgui/backends/imgui_impl_glfw.cpp"
    "${LIB_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
)
target_include_directories(imgui PUBLIC "${LIB_DIR}/imgui" "${LIB_DIR}/imgui/backends" "${LIB_DIR}/glfw/include")
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${SRC_DIR}"
    "${LIB_DIR}/glad/include"
)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

target_compile_definitions(${PROJECT_NAME} PRIVATE J_DEBUG)

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /Zc:preprocessor)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Syncing assets to build output..."
)
