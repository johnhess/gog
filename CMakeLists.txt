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
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.11.0
)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    ohttp_gp
    GIT_REPOSITORY https://github.com/johnhess/ohttp-gp.git
    GIT_TAG origin/hide-boringssl-2
)
FetchContent_MakeAvailable(ohttp_gp)
include_directories(${ohttp_gp_SOURCE_DIR})

# The actual server
add_executable(GogApp app.cc)
target_link_libraries(GogApp PRIVATE Crow::Crow ohttp cpr)

# The server as a testable library
add_library(GogLib app.cc)
target_link_libraries(GogLib PRIVATE Crow::Crow ohttp cpr)

# Testing
add_executable(app_test app_test.cc)
target_link_libraries(app_test PRIVATE gtest gtest_main ohttp GogLib Crow::Crow cpr)
enable_testing()
add_test(NAME app_test COMMAND app_test)