add_executable(hello "main.cpp" "renderables/TriangleRenderable.h" "renderables/TriangleRenderable.cpp")
target_include_directories(hello PRIVATE ${CMAKE_SOURCE_DIR}/drakon/include)
target_link_libraries(hello PRIVATE drakon)

if (WIN32)
  target_link_libraries(hello PRIVATE d3dcompiler.lib)
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET hello PROPERTY CXX_STANDARD 20)
endif()
