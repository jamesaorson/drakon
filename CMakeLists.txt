cmake_minimum_required(VERSION 3.28...4.2)

project(
    exokomodo.drakon # CMake Project Name, which is also the name of the top-level
    # targets (e.g., library, executable, etc.).
    DESCRIPTION "C++ game engine from scratch"
    LANGUAGES CXX
    VERSION 0.1.0
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

if(TARGET glfw)
    target_link_libraries(exokomodo.drakon PRIVATE glfw)
elseif(TARGET glfw::glfw)
    target_link_libraries(exokomodo.drakon PRIVATE glfw::glfw)
else()
    message(FATAL_ERROR "GLFW package was found but did not provide a supported target (glfw or glfw::glfw).")
endif()

target_link_libraries(exokomodo.drakon PRIVATE Vulkan::Vulkan)

message(STATUS "GLFW was found.")
message(STATUS "Vulkan SDK was found.")

file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/drakon/*.cpp")

target_sources(
    exokomodo.drakon
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        ${CPP_SOURCES}
)
target_include_directories(
    exokomodo.drakon
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(exokomodo.drakon PROPERTIES VERIFY_INTERFACE_HEADER_SETS ON)

if(EXOKOMODO_DRAKON_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

if(EXOKOMODO_DRAKON_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
