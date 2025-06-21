cmake_minimum_required(VERSION 3.8)
project(Balatrauto)

set(CMAKE_CXX_STANDARD 17)

# ImGui source files listed explicitly
set(IMGUI_SRC
    "${CMAKE_SOURCE_DIR}/libs/imgui/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/libs/imgui/imgui_demo.cpp"
    "${CMAKE_SOURCE_DIR}/libs/imgui/imgui_draw.cpp"
    "${CMAKE_SOURCE_DIR}/libs/imgui/imgui_tables.cpp"
    "${CMAKE_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp"
    "${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_sdl3.cpp"
    "${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_sdlrenderer3.cpp"
)

# Main source files
set(SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/profile.cpp"
)

add_executable(Balatrauto ${SRC_FILES} ${IMGUI_SRC})

# Include directories
target_include_directories(Balatrauto PRIVATE
    "${CMAKE_SOURCE_DIR}/libs/imgui"
    "${CMAKE_SOURCE_DIR}/libs/imgui/backends"
    "${CMAKE_SOURCE_DIR}/libs/SDL3-3.2.16/include"
    "${CMAKE_SOURCE_DIR}/libs/stb"
    "${CMAKE_SOURCE_DIR}/libs"  # This covers nlohmann and other headers
)

# Link SDL3
target_link_directories(Balatrauto PRIVATE
    "${CMAKE_SOURCE_DIR}/libs/SDL3-3.2.16/lib/x64"
)

target_link_libraries(Balatrauto PRIVATE SDL3.lib)
