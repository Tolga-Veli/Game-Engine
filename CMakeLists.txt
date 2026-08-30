cmake_minimum_required(VERSION 3.20)
project(JupiterEngine VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Single-config generators (Makefiles, Ninja) have an empty CMAKE_BUILD_TYPE
# unless you pass -DCMAKE_BUILD_TYPE explicitly, which silently breaks the
# $<CONFIG:Debug/Release> generator expressions below (neither J_DEBUG nor
# J_RELEASE would get defined). Default to Debug if nothing was specified.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

set(LIB_DIR "${PROJECT_SOURCE_DIR}/libs")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${INCLUDE_DIR}/*.hpp"
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.inl"
)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE "${INCLUDE_DIR}" "${SRC_DIR}")

#---------------------------
# GLFW
#---------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory("${LIB_DIR}/glfw")

#---------------------------
# GLAD 
#---------------------------
add_library(glad STATIC "${LIB_DIR}/glad/src/gl.c")
target_include_directories(glad PUBLIC "${LIB_DIR}/glad/include")

#---------------------------
# GLM 
#---------------------------
target_include_directories(${PROJECT_NAME} PRIVATE "${LIB_DIR}/glm")

#---------------------------
# stb_image 
#---------------------------
target_include_directories(${PROJECT_NAME} PRIVATE "${LIB_DIR}/stb")


#---------------------------
# ImGui (core + GLFW/OpenGL backends) 
#---------------------------
add_library(imgui STATIC
    "${LIB_DIR}/imgui/imgui.cpp"
    "${LIB_DIR}/imgui/imgui_draw.cpp"
    "${LIB_DIR}/imgui/imgui_tables.cpp"
    "${LIB_DIR}/imgui/imgui_widgets.cpp"
    "${LIB_DIR}/imgui/backends/imgui_impl_glfw.cpp"
    "${LIB_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
)
target_include_directories(imgui PUBLIC 
    "${LIB_DIR}/imgui" 
    "${LIB_DIR}/imgui/backends" 
    "${LIB_DIR}/glfw/include"
)

# Link against the real targets instead of hardcoding their include paths -
# glfw/glad already export their own public include dirs, so this stays
# correct even if their internal layout changes.
target_link_libraries(imgui PUBLIC glfw glad)
# Tell imgui_impl_opengl3.cpp we're supplying GLAD2 (gl.h/gl.c naming) as the
# loader, or it falls back to bundling its own minimal loader and fights
# GLAD over who owns the GL function pointers.
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD2)

#---------------------------
# OpenGL
#---------------------------
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad imgui OpenGL::GL)

#---------------------------
# Compiler warnings + optimization
#---------------------------
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /Zc:preprocessor)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/O2>)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)
endif()


#---------------------------
# Build-type defines 
#---------------------------
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:Debug>:J_DEBUG>
    $<$<CONFIG:Release>:J_RELEASE>
)


#---------------------------
# Output
#---------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

#---------------------------
# Assets 
#---------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Syncing assets to build output..."
)
