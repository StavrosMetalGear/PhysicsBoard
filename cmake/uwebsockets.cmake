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

# Link uWebSockets to PhysicsBoard
# (Assuming your target is called PhysicsBoard)
target_link_libraries(PhysicsBoard PRIVATE uWS::uWS)
