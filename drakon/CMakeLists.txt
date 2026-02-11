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

add_library(drakon STATIC ${COMMON_HEADERS} ${COMMON_SOURCES} ${OS_SOURCES})

# Add include directories for the project.

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET drakon PROPERTY CXX_STANDARD 20)
endif()

# Windows

option(DRAKON_USE_GDK "Enable Microsoft GDK (PC)" ON)

if (WIN32 AND DRAKON_USE_GDK)
    if (NOT DEFINED GDK_ROOT)
        set(GDK_ROOT "C:/Program Files (x86)/Microsoft GDK/251001" CACHE PATH "Microsoft GDK root")
    endif()

    set(GDK_GRDK_DIR "${GDK_ROOT}/GRDK")
    set(GDK_GAMEKIT_INCLUDE_DIR "${GDK_GRDK_DIR}/GameKit/Include")
    set(GDK_GAMERUNTIME_INCLUDE_DIR "${GDK_GRDK_DIR}/GameRuntime/Include")
    set(GDK_GAMEKIT_LIB_DIR "${GDK_GRDK_DIR}/GameKit/Lib/amd64")
    set(GDK_GAMERUNTIME_LIB_DIR "${GDK_GRDK_DIR}/GameRuntime/Lib/amd64")

    target_include_directories(drakon PRIVATE ${GDK_GAMEKIT_INCLUDE_DIR} ${GDK_GAMERUNTIME_INCLUDE_DIR})
    target_link_directories(drakon PUBLIC ${GDK_GAMEKIT_LIB_DIR} ${GDK_GAMERUNTIME_LIB_DIR})
    target_link_libraries(drakon PUBLIC GameInput.lib xgameruntime.lib)
    target_compile_definitions(drakon PUBLIC DRAKON_USE_GDK=1)
endif()
