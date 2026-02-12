if (WIN32)
    target_link_libraries(
        exokomodo.drakon
            INTERFACE
            d3d12.lib
            dxgi.lib
            d3dcompiler.lib
        )
endif()
