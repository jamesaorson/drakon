# CMakeList.txt : CMake project for drakon, include source and define
# project specific logic here.
#

# Sources

## Headers
file(GLOB_RECURSE COMMON_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/drakon/*.h")

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

add_library(drakon STATIC ${COMMON_HEADERS} ${COMMON_SOURCES} ${OS_SOURCES} "include/drakon/Renderer.h" "src/drakon/windows/Renderer.cpp" "src/drakon/common/Renderer.cpp" "include/drakon/Renderable.h" )

# Add include directories for the project.

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET drakon PROPERTY CXX_STANDARD 20)
endif()

# Windows
if (WIN32)
    target_link_libraries(drakon PUBLIC d3d12.lib dxgi.lib)
endif()
