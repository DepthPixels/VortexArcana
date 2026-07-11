cmake_minimum_required(VERSION 3.16)
project(VortexArcana VERSION 0.1.0)
project(ScriptingEngineMonolith LANGUAGES CXX CSharp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable building tests/examples for dependencies
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(external/SDL)
add_subdirectory(external/ryml)

# ImGui Setup
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

# Project Sources (Consolidated for easier maintenance)
set(VORTEX_SOURCES
    src/main.cpp
    src/Core/Engine.cpp
    src/Core/Engine.h
    src/Core/Entity.cpp
    src/Core/Entity.h
    src/Core/Component.h
    src/Core/Utility/Math.h
    src/Core/Utility/Shader.h
    src/Core/Utility/Textures.cpp
    src/Core/Utility/Textures.h
    src/Core/Utility/SceneUtility.cpp
    src/Core/Utility/SceneUtility.h
    src/Core/Utility/ScriptingInterface.h
    src/Core/Components/SpriteRenderer2D.cpp
    src/Core/Components/Physics2D.cpp
    src/Core/Components/Physics2D.h
    src/Core/Entities/Tile.h
    external/glad/src/glad.c
    assets/shaders/TexFragment.glsl
    assets/shaders/TexVertex.glsl
)

add_executable(VortexArcana ${VORTEX_SOURCES} ${IMGUI_SOURCES})

# Fix VS Hot Reload
if(MSVC)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")
    
    add_compile_options(/Zi /Od)
    add_link_options(/INCREMENTAL:NO)
endif()

target_include_directories(VortexArcana PRIVATE 
    src/
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    external/glad/include
    external/stb_image
    external/glm
    external/nethost/include
)

target_link_libraries(VortexArcana PRIVATE 
    SDL3::SDL3 
    ryml::ryml
    ${CMAKE_CURRENT_SOURCE_DIR}/external/nethost/lib/nethost.lib
)

# Copy SDL3 DLL
add_custom_command(TARGET VortexArcana POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:SDL3::SDL3>"
    "$<TARGET_FILE_DIR:VortexArcana>"
)

# Copy nethost dll
add_custom_command(TARGET VortexArcana POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/external/nethost/lib/nethost.dll"
    "$<TARGET_FILE_DIR:VortexArcana>"
)

# Copy the assets folder to the build directory automatically
add_custom_command(TARGET VortexArcana POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:VortexArcana>/assets"
)

# CSharp Stuff

add_custom_target(ScriptHandler ALL
    COMMAND dotnet build "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/CSharp/ScriptHandler.csproj" --configuration $<CONFIG> --output "${CMAKE_CURRENT_BINARY_DIR}/csharp_out"
    COMMENT "Compiling C# Script Handler natively via .NET 10 CLI..."
)

add_dependencies(VortexArcana ScriptHandler)

add_custom_command(TARGET VortexArcana POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_BINARY_DIR}/csharp_out"
        "$<TARGET_FILE_DIR:VortexArcana>"
    COMMENT "Deploying .NET 10 binaries into the VortexArcana runtime directory..."
)

# Replace standard include_directories with SYSTEM to suppress their warnings
target_include_directories(VortexArcana SYSTEM PRIVATE external/)


