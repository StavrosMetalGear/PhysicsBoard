# Download and add uWebSockets (header-only, static)

include(FetchContent)
FetchContent_Declare(
    uwebsockets
    GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git
    GIT_TAG v20.62.0
)
FetchContent_MakeAvailable(uwebsockets)

# Add uWebSockets include directories
include_directories(${uwebsockets_SOURCE_DIR}/src)

# Find uWebSockets sources
file(GLOB UWS_SOURCES
    ${uwebsockets_SOURCE_DIR}/src/*.cpp
)

add_library(uwebsockets STATIC ${UWS_SOURCES})
target_include_directories(uwebsockets PUBLIC
    ${uwebsockets_SOURCE_DIR}/src
    ${uwebsockets_SOURCE_DIR}/deps
)
target_compile_definitions(uwebsockets PUBLIC UWS_NO_ZLIB)
target_link_libraries(uwebsockets PUBLIC ssl crypto z)

# Link uWebSockets to PhysicsBoard
# (Assuming your target is called PhysicsBoard)
#find_package(uwebsockets CONFIG REQUIRED)
target_link_libraries(PhysicsBoard PRIVATE imgui_lib implot_lib OpenGL::GL uwebsockets)

