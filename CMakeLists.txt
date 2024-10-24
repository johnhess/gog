cmake_minimum_required(VERSION 3.14)
project(Gog)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.2.0
)
FetchContent_MakeAvailable(crow)

FetchContent_Declare(
    ohttp_gp
    GIT_REPOSITORY https://github.com/johnhess/ohttp-gp.git
    GIT_TAG origin/main
)
FetchContent_MakeAvailable(ohttp_gp)
include_directories(${ohttp_gp_SOURCE_DIR})

add_executable(GogApp app.cc)

target_link_libraries(GogApp PRIVATE Crow::Crow ohttp)