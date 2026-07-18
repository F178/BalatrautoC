cmake_minimum_required(VERSION 3.20)
project(Balatrauto LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/libs/imgui")
set(SDL3_DIR "${CMAKE_SOURCE_DIR}/libs/SDL3-3.2.16")

add_executable(Balatrauto
    src/main.cpp
    src/profile.cpp
    src/ui_helpers.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(Balatrauto PRIVATE
    src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${SDL3_DIR}/include
    ${CMAKE_SOURCE_DIR}/libs/stb
    ${CMAKE_SOURCE_DIR}/libs
)

target_link_directories(Balatrauto PRIVATE
    ${SDL3_DIR}/lib/x64
)

target_link_libraries(Balatrauto PRIVATE SDL3.lib)

set_property(TARGET Balatrauto PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

if(WIN32)
    add_custom_command(TARGET Balatrauto POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL3_DIR}/lib/x64/SDL3.dll"
            "$<TARGET_FILE_DIR:Balatrauto>"
    )
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/Icons")
    add_custom_command(TARGET Balatrauto POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Icons"
            "$<TARGET_FILE_DIR:Balatrauto>/Icons"
    )
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    add_custom_command(TARGET Balatrauto POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:Balatrauto>/assets"
    )
endif()
