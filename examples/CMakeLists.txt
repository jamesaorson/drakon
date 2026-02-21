if(WIN32)
    set(
        ALL_EXAMPLES
        hello-directx
    )
endif()

if(EXOKOMODO_DRAKON_HAS_VULKAN_BACKEND)
    set(
        ALL_EXAMPLES
        hello-vulkan
    )
endif()

message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(exokomodo.drakon.examples.${example})
    set_property(TARGET exokomodo.drakon.examples.${example} PROPERTY CXX_STANDARD 20)
    target_sources(exokomodo.drakon.examples.${example} PRIVATE ${example}/main.cpp)
    target_link_libraries(
        exokomodo.drakon.examples.${example}
        PRIVATE
        exokomodo::drakon
    )
endforeach()
