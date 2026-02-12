add_executable(exokomodo.drakon.tests.stub stub.cpp)

set_property(TARGET exokomodo.drakon.tests.stub PROPERTY CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

target_link_libraries(
    exokomodo.drakon.tests.stub
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(exokomodo.drakon.tests.stub)
