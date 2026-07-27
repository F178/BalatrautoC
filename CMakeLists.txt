cmake_minimum_required(VERSION 3.20)
project(Balatrauto VERSION 0.6.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(BALATRAUTO_LOCAL_DEPS
    "${CMAKE_SOURCE_DIR}/libs/imgui/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/libs/SDL3-3.2.16/include/SDL3/SDL.h"
    "${CMAKE_SOURCE_DIR}/libs/SDL3-3.2.16/lib/x64/SDL3.lib"
    "${CMAKE_SOURCE_DIR}/libs/nlohmann/json.hpp"
    "${CMAKE_SOURCE_DIR}/libs/stb/stb_image.h"
)

set(BALATRAUTO_USE_LOCAL_DEPS TRUE)
foreach(dependency IN LISTS BALATRAUTO_LOCAL_DEPS)
    if(NOT EXISTS "${dependency}")
        set(BALATRAUTO_USE_LOCAL_DEPS FALSE)
    endif()
endforeach()

if(BALATRAUTO_USE_LOCAL_DEPS)
    set(BALATRAUTO_IMGUI_DIR "${CMAKE_SOURCE_DIR}/libs/imgui")
    set(BALATRAUTO_STB_DIR "${CMAKE_SOURCE_DIR}/libs/stb")
    set(BALATRAUTO_JSON_DIR "${CMAKE_SOURCE_DIR}/libs")
    set(BALATRAUTO_SDL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/SDL3-3.2.16/include")
    add_library(BalatrautoSDL3 SHARED IMPORTED GLOBAL)
    set_target_properties(BalatrautoSDL3 PROPERTIES
        IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/libs/SDL3-3.2.16/lib/x64/SDL3.lib"
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libs/SDL3-3.2.16/lib/x64/SDL3.dll"
    )
    set(BALATRAUTO_SDL_TARGET BalatrautoSDL3)
else()
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.2.16
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(SDL3)

    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.7
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(imgui)

    set(JSON_BuildTests OFF CACHE INTERNAL "")
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)

    FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG 31c1ad37456438565541f4919958214b6e762fb4
    )
    FetchContent_MakeAvailable(stb)

    set(BALATRAUTO_IMGUI_DIR "${imgui_SOURCE_DIR}")
    set(BALATRAUTO_STB_DIR "${stb_SOURCE_DIR}")
    set(BALATRAUTO_JSON_DIR "${nlohmann_json_SOURCE_DIR}/include")
    set(BALATRAUTO_SDL_INCLUDE_DIR "${SDL3_SOURCE_DIR}/include")
    set(BALATRAUTO_SDL_TARGET SDL3::SDL3)
endif()

set(BALATRAUTO_ICONS_DIR "${CMAKE_SOURCE_DIR}/Icons")
if(NOT EXISTS "${BALATRAUTO_ICONS_DIR}" AND EXISTS "${CMAKE_SOURCE_DIR}/Icons.zip")
    set(BALATRAUTO_RUNTIME_ASSETS "${CMAKE_BINARY_DIR}/runtime-assets")
    file(MAKE_DIRECTORY "${BALATRAUTO_RUNTIME_ASSETS}")
    file(ARCHIVE_EXTRACT INPUT "${CMAKE_SOURCE_DIR}/Icons.zip" DESTINATION "${BALATRAUTO_RUNTIME_ASSETS}")
    set(BALATRAUTO_ICONS_DIR "${BALATRAUTO_RUNTIME_ASSETS}/Icons")
endif()

set(IMGUI_SRC
    "${BALATRAUTO_IMGUI_DIR}/imgui.cpp"
    "${BALATRAUTO_IMGUI_DIR}/imgui_draw.cpp"
    "${BALATRAUTO_IMGUI_DIR}/imgui_tables.cpp"
    "${BALATRAUTO_IMGUI_DIR}/imgui_widgets.cpp"
    "${BALATRAUTO_IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"
    "${BALATRAUTO_IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp"
)

set(SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/app_paths.cpp"
    "${CMAKE_SOURCE_DIR}/src/game_asset_catalog.cpp"
    "${CMAKE_SOURCE_DIR}/src/image_matcher.cpp"
    "${CMAKE_SOURCE_DIR}/src/image_region_detector.cpp"
    "${CMAKE_SOURCE_DIR}/src/profile.cpp"
    "${CMAKE_SOURCE_DIR}/src/seed_engine.cpp"
    "${CMAKE_SOURCE_DIR}/src/seed_lab.cpp"
    "${CMAKE_SOURCE_DIR}/src/seed_session.cpp"
    "${CMAKE_SOURCE_DIR}/src/texture_cache.cpp"
    "${CMAKE_SOURCE_DIR}/src/tracker_app.cpp"
    "${CMAKE_SOURCE_DIR}/src/ui_helpers.cpp"
    "${CMAKE_SOURCE_DIR}/src/stb_image_impl.cpp"
    "${CMAKE_SOURCE_DIR}/src/window_capture.cpp"
)

if(MSVC)
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

add_executable(Balatrauto WIN32 ${SRC_FILES} ${IMGUI_SRC})

target_include_directories(Balatrauto PRIVATE
    "${BALATRAUTO_IMGUI_DIR}"
    "${BALATRAUTO_IMGUI_DIR}/backends"
    "${BALATRAUTO_SDL_INCLUDE_DIR}"
    "${BALATRAUTO_STB_DIR}"
    "${BALATRAUTO_JSON_DIR}"
    "${CMAKE_SOURCE_DIR}/src"
)

target_include_directories(Balatrauto SYSTEM PRIVATE
    "${CMAKE_SOURCE_DIR}/third_party/balatro-seed-finder"
)

target_link_libraries(Balatrauto PRIVATE ${BALATRAUTO_SDL_TARGET})
if(NOT BALATRAUTO_USE_LOCAL_DEPS)
    target_link_libraries(Balatrauto PRIVATE nlohmann_json::nlohmann_json)
endif()
if(WIN32)
    target_link_libraries(Balatrauto PRIVATE user32.lib gdi32.lib)
endif()
target_compile_definitions(Balatrauto PRIVATE
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
    BALATRAUTO_VERSION="${PROJECT_VERSION}"
)

if(MSVC)
    target_compile_options(Balatrauto PRIVATE /W4 /permissive- /external:W0)
endif()

add_custom_command(TARGET Balatrauto POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${BALATRAUTO_SDL_TARGET}>"
        "$<TARGET_FILE_DIR:Balatrauto>/SDL3.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${BALATRAUTO_ICONS_DIR}"
        "$<TARGET_FILE_DIR:Balatrauto>/Icons"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:Balatrauto>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/THIRD_PARTY_NOTICES.md"
        "$<TARGET_FILE_DIR:Balatrauto>/THIRD_PARTY_NOTICES.md"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/third_party/balatro-seed-finder/LICENSE"
        "$<TARGET_FILE_DIR:Balatrauto>/BALATRO_SEED_FINDER_LICENSE.txt"
    VERBATIM
)

include(CTest)

if(BUILD_TESTING)
    add_executable(BalatrautoTests
        "${CMAKE_SOURCE_DIR}/tests/profile_tests.cpp"
        "${CMAKE_SOURCE_DIR}/src/profile.cpp"
    )
    target_include_directories(BalatrautoTests PRIVATE
        "${BALATRAUTO_JSON_DIR}"
        "${CMAKE_SOURCE_DIR}/src"
    )
    target_compile_definitions(BalatrautoTests PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
    if(MSVC)
        target_compile_options(BalatrautoTests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME profile_round_trip COMMAND BalatrautoTests)

    add_executable(BalatrautoSeedTests
        "${CMAKE_SOURCE_DIR}/tests/seed_engine_tests.cpp"
        "${CMAKE_SOURCE_DIR}/src/seed_engine.cpp"
        "${CMAKE_SOURCE_DIR}/src/seed_session.cpp"
    )
    target_include_directories(BalatrautoSeedTests PRIVATE
        "${BALATRAUTO_JSON_DIR}"
        "${CMAKE_SOURCE_DIR}/libs"
        "${CMAKE_SOURCE_DIR}/src"
    )
    target_include_directories(BalatrautoSeedTests SYSTEM PRIVATE
        "${CMAKE_SOURCE_DIR}/third_party/balatro-seed-finder"
    )
    target_compile_definitions(BalatrautoSeedTests PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
    if(MSVC)
        target_compile_options(BalatrautoSeedTests PRIVATE /W4 /permissive- /fp:strict /external:W0)
    endif()
    add_test(NAME seed_engine COMMAND BalatrautoSeedTests)

    if(EXISTS "${CMAKE_SOURCE_DIR}/assets/game_reference/asset_manifest.json")
        add_executable(BalatrautoAssetTests
        "${CMAKE_SOURCE_DIR}/tests/asset_catalog_tests.cpp"
        "${CMAKE_SOURCE_DIR}/src/game_asset_catalog.cpp"
        )
        target_include_directories(BalatrautoAssetTests PRIVATE
            "${BALATRAUTO_JSON_DIR}"
            "${CMAKE_SOURCE_DIR}/src"
        )
        target_compile_definitions(BalatrautoAssetTests PRIVATE
            NOMINMAX
            _CRT_SECURE_NO_WARNINGS
            BALATRAUTO_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        )
        if(MSVC)
            target_compile_options(BalatrautoAssetTests PRIVATE /W4 /permissive-)
        endif()
        add_test(NAME asset_catalog COMMAND BalatrautoAssetTests)

        add_executable(BalatrautoImageMatcherTests
        "${CMAKE_SOURCE_DIR}/tests/image_matcher_tests.cpp"
        "${CMAKE_SOURCE_DIR}/src/image_matcher.cpp"
        "${CMAKE_SOURCE_DIR}/src/stb_image_impl.cpp"
        )
        target_include_directories(BalatrautoImageMatcherTests PRIVATE
            "${BALATRAUTO_JSON_DIR}"
            "${BALATRAUTO_STB_DIR}"
            "${CMAKE_SOURCE_DIR}/src"
        )
        target_compile_definitions(BalatrautoImageMatcherTests PRIVATE
            NOMINMAX
            _CRT_SECURE_NO_WARNINGS
            BALATRAUTO_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        )
        if(MSVC)
            target_compile_options(BalatrautoImageMatcherTests PRIVATE /W4 /permissive-)
        endif()
        add_test(NAME image_matcher COMMAND BalatrautoImageMatcherTests)

        add_executable(BalatrautoImageRegionTests
        "${CMAKE_SOURCE_DIR}/tests/image_region_detector_tests.cpp"
        "${CMAKE_SOURCE_DIR}/src/image_region_detector.cpp"
        "${CMAKE_SOURCE_DIR}/src/stb_image_impl.cpp"
        )
        target_include_directories(BalatrautoImageRegionTests PRIVATE
            "${BALATRAUTO_STB_DIR}"
            "${CMAKE_SOURCE_DIR}/src"
        )
        target_compile_definitions(BalatrautoImageRegionTests PRIVATE
            NOMINMAX
            _CRT_SECURE_NO_WARNINGS
            BALATRAUTO_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
        )
        if(MSVC)
            target_compile_options(BalatrautoImageRegionTests PRIVATE /W4 /permissive-)
        endif()
        add_test(NAME image_region_detector COMMAND BalatrautoImageRegionTests)
    endif()
endif()

install(TARGETS Balatrauto RUNTIME DESTINATION .)
install(FILES
    "$<TARGET_FILE:${BALATRAUTO_SDL_TARGET}>"
    "${CMAKE_SOURCE_DIR}/THIRD_PARTY_NOTICES.md"
    DESTINATION .
)
install(FILES
    "${CMAKE_SOURCE_DIR}/third_party/balatro-seed-finder/LICENSE"
    DESTINATION .
    RENAME BALATRO_SEED_FINDER_LICENSE.txt
)
install(DIRECTORY "${BALATRAUTO_ICONS_DIR}" DESTINATION .)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets" DESTINATION .)

set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_NAME Balatrauto)
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "Balatrauto-${PROJECT_VERSION}-windows-x64")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)
