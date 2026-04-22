cmake_minimum_required(VERSION 3.15)
project(PhysicsBoard LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# GLFW
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

# Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.8
)
FetchContent_MakeAvailable(imgui)

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_lib PUBLIC glfw)

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

find_package(OpenGL REQUIRED)

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

add_executable(
    PhysicsBoard
    ${COMMON_SOURCES}
    ${COMMON_HEADERS}
    ${PLATFORM_SOURCES}
    ${PLATFORM_HEADERS}
)

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
    OpenGL::GL
    zlib
    nlohmann_json::nlohmann_json
)

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