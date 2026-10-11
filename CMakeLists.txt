cmake_minimum_required(VERSION 3.15)
project(PhysicsBoard LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

option(USE_SDL "Enable SDL2 backend and builds (for Android/mobile)" OFF)

# GLFW (desktop only)
if(NOT ANDROID)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
    )
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

# Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.8
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_BACKENDS)
list(APPEND IMGUI_BACKENDS ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
if(NOT ANDROID)
    list(APPEND IMGUI_BACKENDS ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)
endif()
if(ANDROID OR USE_SDL)
    list(APPEND IMGUI_BACKENDS ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp)
endif()

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${IMGUI_BACKENDS}
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
if(NOT ANDROID)
    target_link_libraries(imgui_lib PUBLIC glfw)
endif()

# If SDL is requested (Android or explicitly), fetch SDL2 and link where needed
if(ANDROID OR USE_SDL)
    FetchContent_Declare(
        sdl2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.28.6
    )
    FetchContent_MakeAvailable(sdl2)
    # imgui_impl_sdl.cpp will use SDL2; link SDL2 to final target when needed
endif()

# ImPlot
FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG        v0.16
)
FetchContent_MakeAvailable(implot)

add_library(implot_lib STATIC
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
)
target_include_directories(implot_lib PUBLIC ${implot_SOURCE_DIR})
target_link_libraries(implot_lib PUBLIC imgui_lib)

# zlib
FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG        v1.3.1
)
FetchContent_MakeAvailable(zlib)

# nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

if(NOT ANDROID)
    find_package(OpenGL REQUIRED)
else()
    # On Android we'll link against OpenGLES provided by the NDK
    find_library(GLESv2_LIB GLESv2)
endif()

# Only fetch/build uWebSockets on UNIX
if(UNIX)
    FetchContent_Declare(
        uwebsockets
        GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git
        GIT_TAG        v20.62.0
        GIT_SUBMODULES uSockets
    )
    FetchContent_MakeAvailable(uwebsockets)

    set(USOCKETS_DIR ${uwebsockets_SOURCE_DIR}/uSockets/src)

    message(STATUS "Configuring uSockets for UNIX/POSIX with epoll backend")

    add_library(usockets STATIC
        ${USOCKETS_DIR}/context.c
        ${USOCKETS_DIR}/loop.c
        ${USOCKETS_DIR}/socket.c
        ${USOCKETS_DIR}/udp.c
        ${USOCKETS_DIR}/bsd.c
        ${USOCKETS_DIR}/eventing/epoll_kqueue.c
    )

    target_include_directories(usockets PUBLIC
        ${USOCKETS_DIR}
    )

    target_compile_definitions(usockets PRIVATE
        LIBUS_USE_EPOLL
        LIBUS_NO_SSL
    )
endif()

set(COMMON_SOURCES
    main.cpp
    BoardApp.cpp
    Board.cpp
    FunctionPlotter.cpp
    ExpressionParser.cpp
    NetworkSession.cpp
    ProjectileSimulator.cpp
    PhysicsToolbox.cpp
    BoardAppWebSocket.cpp
)

if(ANDROID)
    # Android uses a separate main that integrates with SDL and OpenGLES
    list(APPEND COMMON_SOURCES main_android.cpp)
elseif(USE_SDL)
    # Desktop builds can also opt into SDL main (not added by default)
    # list(APPEND COMMON_SOURCES main_android.cpp)
endif()

set(COMMON_HEADERS
    BoardApp.h
    Board.h
    FunctionPlotter.h
    ExpressionParser.h
    NetworkSession.h
    ProjectileSimulator.h
    PhysicsToolbox.h
    IWebSocketServer.h
)

if(WIN32)
    set(PLATFORM_SOURCES
        WinWebSocketServer.cpp
    )
    set(PLATFORM_HEADERS
        WinWebSocketServer.h
    )
elseif(UNIX)
    set(PLATFORM_SOURCES
        UwsWebSocketServer.cpp
    )
    set(PLATFORM_HEADERS
        UwsWebSocketServer.h
    )
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

if(ANDROID)
    add_library(
        PhysicsBoard SHARED
        ${COMMON_SOURCES}
        ${COMMON_HEADERS}
        ${PLATFORM_SOURCES}
        ${PLATFORM_HEADERS}
    )
    # Ensure position independent code for shared library
    set_target_properties(PhysicsBoard PROPERTIES POSITION_INDEPENDENT_CODE ON)
else()
    add_executable(
        PhysicsBoard
        ${COMMON_SOURCES}
        ${COMMON_HEADERS}
        ${PLATFORM_SOURCES}
        ${PLATFORM_HEADERS}
    )
endif()

target_include_directories(PhysicsBoard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(UNIX)
    target_include_directories(PhysicsBoard PRIVATE
        ${uwebsockets_SOURCE_DIR}/src
        ${uwebsockets_SOURCE_DIR}/uSockets/src
    )
endif()

target_link_libraries(PhysicsBoard PRIVATE
    imgui_lib
    implot_lib
    zlib
    nlohmann_json::nlohmann_json
)

if(ANDROID)
    target_link_libraries(PhysicsBoard PRIVATE ${GLESv2_LIB})
else()
    target_link_libraries(PhysicsBoard PRIVATE OpenGL::GL)
endif()

if(ANDROID OR USE_SDL)
    target_link_libraries(PhysicsBoard PRIVATE SDL2::SDL2)
endif()

if(WIN32)
    target_compile_definitions(PhysicsBoard PRIVATE
        _USE_MATH_DEFINES
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )
    target_compile_options(PhysicsBoard PRIVATE /W3)
    target_link_libraries(PhysicsBoard PRIVATE ws2_32)

elseif(UNIX)
    target_compile_definitions(PhysicsBoard PRIVATE
        LIBUS_USE_EPOLL
        LIBUS_NO_SSL
    )
    target_compile_options(PhysicsBoard PRIVATE -Wall -Wextra -pedantic)
    target_link_libraries(PhysicsBoard PRIVATE usockets)
endif()