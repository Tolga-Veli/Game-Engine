cmake_minimum_required(VERSION 3.20)
project(Jupiter VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(JUPITER_BUILD_EXAMPLE "Build the default Jupiter example application" ON)
option(JUPITER_BUILD_PLANET_EXAMPLE "Build the planet simulation example" ON)

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

file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS
    "${INCLUDE_DIR}/*.hpp"
)

file(GLOB_RECURSE INL_FILES CONFIGURE_DEPENDS
  "${SRC_DIR}/*.inl"
)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${SRC_DIR}/*.cpp"
)

add_library(JupiterEngine STATIC ${SRC_FILES} ${HEADER_FILES} ${INL_FILES})
target_sources(JupiterEngine PRIVATE "${INCLUDE_DIR}/Core/GLFW/GLFW_Window.cpp")

target_include_directories(JupiterEngine
    PUBLIC "${INCLUDE_DIR}"
    PRIVATE "${SRC_DIR}"
)

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
target_include_directories(JupiterEngine PUBLIC "${LIB_DIR}/glm")

#---------------------------
# stb_image 
#---------------------------
target_include_directories(JupiterEngine PUBLIC "${LIB_DIR}/stb")


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

target_link_libraries(imgui PUBLIC glfw glad)

target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD2)

#---------------------------
# OpenGL
#---------------------------
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(JupiterEngine
    PUBLIC glfw glad imgui OpenGL::GL Threads::Threads
)

#---------------------------
# Compiler warnings + optimization
#---------------------------
function(jupiter_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /Zc:preprocessor)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic)
    endif()
endfunction()

jupiter_enable_warnings(JupiterEngine)


if(MSVC)
    target_compile_options(JupiterEngine PRIVATE $<$<CONFIG:Release>:/O2>)
else()
    target_compile_options(JupiterEngine PRIVATE $<$<CONFIG:Release>:-O3>)
endif()

if(JUPITER_BUILD_PLANET_EXAMPLE)
    add_executable(PlanetRendererExample
        "${PROJECT_SOURCE_DIR}/examples/PlanetRenderer/main.cpp"
        "${PROJECT_SOURCE_DIR}/examples/PlanetRenderer/PlanetRenderer.cpp"
        "${PROJECT_SOURCE_DIR}/examples/PlanetRenderer/Simulation.cpp")
    target_link_libraries(PlanetRendererExample PRIVATE JupiterEngine)
    target_include_directories(PlanetRendererExample PRIVATE "${PROJECT_SOURCE_DIR}/examples/PlanetRenderer")
    jupiter_enable_warnings(PlanetRendererExample)
    set_target_properties(PlanetRendererExample PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    add_custom_command(TARGET PlanetRendererExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:PlanetRendererExample>/assets")
endif()


#---------------------------
# Build-type defines 
#---------------------------
target_compile_definitions(JupiterEngine PRIVATE 
    JUPITER_SOURCE_ASSET_DIR="${PROJECT_SOURCE_DIR}/assets"
    $<$<CONFIG:Debug>:J_DEBUG>
    $<$<CONFIG:Release>:J_RELEASE>
)


#---------------------------
# Output
#---------------------------
set_target_properties(JupiterEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

#---------------------------
# Example project 
#---------------------------
if(JUPITER_BUILD_EXAMPLE)
    add_executable(JupiterExample
        "${PROJECT_SOURCE_DIR}/examples/RotatingCube/main.cpp"
    )

    target_link_libraries(JupiterExample
        PRIVATE JupiterEngine
    )
    jupiter_enable_warnings(JupiterExample)

    set_target_properties(JupiterExample PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    #---------------------------
    # Assets 
    #---------------------------
    add_custom_command(TARGET JupiterExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:JupiterExample>/assets"
        COMMENT "Syncing assets to build output..."
    )
endif()
