cmake_minimum_required(VERSION 3.28...4.2)

project(
    exokomodo.drakon # CMake Project Name, which is also the name of the top-level
    # targets (e.g., library, executable, etc.).
    DESCRIPTION "C++ game engine from scratch"
    LANGUAGES CXX
    VERSION 0.1.0
)

option(
    EXOKOMODO_DRAKON_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

option(
    EXOKOMODO_DRAKON_BUILD_EXAMPLES
    "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

add_library(exokomodo.drakon)
add_library(exokomodo::drakon ALIAS exokomodo.drakon)

set_property(TARGET exokomodo.drakon PROPERTY CXX_STANDARD 20)

## Common Sources
file(GLOB_RECURSE COMMON_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/drakon/common/*.cpp")

## OS Sources
if(WIN32)
    file(GLOB_RECURSE OS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/drakon/windows/*.cpp")
elseif(APPLE)
    file(GLOB_RECURSE OS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/drakon/darwin/*.cpp")
elseif(UNIX)
    file(GLOB_RECURSE OS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/drakon/linux/*.cpp")
else()
    message(FATAL_ERROR "Unsupported operating system")
endif()
target_sources(
    exokomodo.drakon
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        ${COMMON_SOURCES} ${OS_SOURCES}
)
target_include_directories(
    exokomodo.drakon
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)


set_target_properties(exokomodo.drakon PROPERTIES VERIFY_INTERFACE_HEADER_SETS ON)

add_subdirectory(include/drakon)

if(EXOKOMODO_DRAKON_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

if(EXOKOMODO_DRAKON_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
