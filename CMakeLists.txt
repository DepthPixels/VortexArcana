cmake_minimum_required(VERSION 3.16)
project(VortexArcana VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(external/SDL)

set(IMGUI_DIR external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)

add_executable(VortexArcana 
    src/main.cpp 
    src/Core/Engine.cpp 
    ${IMGUI_SOURCES}
    src/Core/Math.h
    src/Core/Entity.h "src/Core/Shader.h")

# Fix VS Hot Reload
if(MSVC)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")
    
    add_compile_options(/Zi /Od)
    add_link_options(/INCREMENTAL:NO)
endif()

target_sources(VortexArcana PRIVATE 
    src/main.cpp
    external/glad/src/glad.c
)

target_include_directories(VortexArcana PRIVATE 
    src/
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    external/glad/include
)

target_link_libraries(VortexArcana PRIVATE SDL3::SDL3)

add_custom_command(TARGET VortexArcana POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:SDL3::SDL3>"
    "$<TARGET_FILE_DIR:VortexArcana>"
)

# Copy the assets folder to the build directory automatically
add_custom_command(TARGET VortexArcana POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:VortexArcana>/assets"
)
