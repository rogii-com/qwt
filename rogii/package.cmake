if(TARGET qwt::library)
    return()
endif()

add_library(qwt::library SHARED IMPORTED)

if(MSVC)
    set_target_properties(
        qwt::library
        PROPERTIES
            IMPORTED_LOCATION
                "${CMAKE_CURRENT_LIST_DIR}/bin/qwt.dll"
            IMPORTED_LOCATION_DEBUG
                "${CMAKE_CURRENT_LIST_DIR}/bin/qwtd.dll"
            IMPORTED_IMPLIB
                "${CMAKE_CURRENT_LIST_DIR}/bin/qwt.lib"
            IMPORTED_IMPLIB_DEBUG
                "${CMAKE_CURRENT_LIST_DIR}/bin/qwtd.lib"
            INTERFACE_INCLUDE_DIRECTORIES
                "${CMAKE_CURRENT_LIST_DIR}/include/"
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_target_properties(
        qwt::library
        PROPERTIES
            IMPORTED_LOCATION
                "${CMAKE_CURRENT_LIST_DIR}/bin/libqwt.so"
            IMPORTED_LOCATION_DEBUG
                "${CMAKE_CURRENT_LIST_DIR}/bin/libqwtd.so"
            INTERFACE_INCLUDE_DIRECTORIES
                "${CMAKE_CURRENT_LIST_DIR}/include/"
        )
endif()

set(
    COMPONENT_NAMES

    CNPM_RUNTIME_qwt_library
    CNPM_RUNTIME
)

foreach(COMPONENT_NAME ${COMPONENT_NAMES})
    install(
        FILES
            $<TARGET_FILE:qwt::library>
        DESTINATION
            .
        COMPONENT
            ${COMPONENT_NAME}
        EXCLUDE_FROM_ALL
    )
endforeach()