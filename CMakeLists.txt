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

set(EXOKOMODO_DRAKON_HAS_VULKAN_BACKEND OFF)

if(NOT WIN32)
    # Find GLFW
    find_package(glfw3 QUIET)
    if(TARGET glfw)
        target_link_libraries(exokomodo.drakon PRIVATE glfw)
        target_compile_definitions(exokomodo.drakon PRIVATE DRAKON_HAS_GLFW)
        set(EXOKOMODO_DRAKON_HAS_GLFW ON)
    elseif(TARGET glfw::glfw)
        target_link_libraries(exokomodo.drakon PRIVATE glfw::glfw)
        target_compile_definitions(exokomodo.drakon PRIVATE DRAKON_HAS_GLFW)
        set(EXOKOMODO_DRAKON_HAS_GLFW ON)
    endif()
    if(EXOKOMODO_DRAKON_HAS_GLFW)
        message(STATUS "GLFW was found.")
    else()
        message(STATUS "GLFW was not found.")
    endif()

    # Find Vulkan
    find_package(Vulkan QUIET)
    if(Vulkan_FOUND)
        target_link_libraries(exokomodo.drakon PRIVATE Vulkan::Vulkan)
        target_compile_definitions(exokomodo.drakon PUBLIC DRAKON_HAS_VULKAN)
        set(EXOKOMODO_DRAKON_HAS_VULKAN_BACKEND ON)
    endif()
    if(EXOKOMODO_DRAKON_HAS_VULKAN_BACKEND)
        message(STATUS "Vulkan SDK was found.")
    else()
        message(STATUS "Vulkan SDK was not found.")
    endif()
endif()

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

if(EXOKOMODO_DRAKON_HAS_VULKAN_BACKEND)
    file(GLOB_RECURSE VULKAN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/drakon/vulkan/*.cpp")
    set(OS_SOURCES ${OS_SOURCES} ${VULKAN_SOURCES})
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
