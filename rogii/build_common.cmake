if(
    NOT DEFINED ROOT
    OR NOT DEFINED ARCH
)
    message(
        FATAL_ERROR
        "Assert: ROOT = ${ROOT}; ARCH = ${ARCH}"
    )
endif()

set(
    BUILD
    0
)

if(DEFINED ENV{BUILD_NUMBER})
    set(
        BUILD
        $ENV{BUILD_NUMBER}
    )
endif()

set(
    TAG
    ""
)

if(DEFINED ENV{TAG})
    set(
        TAG
        "$ENV{TAG}"
    )
else()
    find_package(
        Git
    )

    if(Git_FOUND)
        execute_process(
            COMMAND
                ${GIT_EXECUTABLE} rev-parse --short HEAD
            OUTPUT_VARIABLE
                TAG
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(
            TAG
            "_${TAG}"
        )
    endif()
endif()

include(
    "${CMAKE_CURRENT_LIST_DIR}/version.cmake"
)

set(
    DEBUG_PATH
    "${CMAKE_CURRENT_LIST_DIR}/../build_debug"
)
set(
    RELEASE_PATH
    "${CMAKE_CURRENT_LIST_DIR}/../build_release"
)

set(
    PACKAGE_NAME
    "qwt-${ROGII_PKG_VERSION}-${ARCH}-${BUILD}${TAG}"
)

set(
    CMAKE_INSTALL_PREFIX
    ${ROOT}/${PACKAGE_NAME}
)

file(
    MAKE_DIRECTORY
    "${DEBUG_PATH}"
)

execute_process(
    COMMAND
        "${CMAKE_COMMAND}" -DCMAKE_INSTALL_PREFIX=${ROOT}/${PACKAGE_NAME} -DCMAKE_BUILD_TYPE=Debug -G Ninja ..
    WORKING_DIRECTORY
        "${DEBUG_PATH}"
)

execute_process(
    COMMAND
        "${CMAKE_COMMAND}" --build . --target install
    WORKING_DIRECTORY
        "${DEBUG_PATH}"
)

file(
    MAKE_DIRECTORY
    "${RELEASE_PATH}"
)

# execute_process(
#     COMMAND
#         "${CMAKE_COMMAND}" -DCMAKE_INSTALL_PREFIX=${ROOT}/${PACKAGE_NAME} -DCMAKE_BUILD_TYPE=RelWithDebInfo -G Ninja ..
#     WORKING_DIRECTORY
#         "${RELEASE_PATH}"
# )

# execute_process(
#     COMMAND
#         "${CMAKE_COMMAND}" --build . --target install
#     WORKING_DIRECTORY
#         "${RELEASE_PATH}"
# )

file(
    COPY
        "${CMAKE_CURRENT_LIST_DIR}/package.cmake"
    DESTINATION
        "${CMAKE_INSTALL_PREFIX}"
)

if(UNIX)
    #strip all shared not symlink libs
    file(GLOB files "${CMAKE_INSTALL_PREFIX}/bin/*.so*")
    foreach(file ${files})
        if(NOT IS_SYMLINK ${file})
            message(STATUS "strip ${file}")
            execute_process(
                COMMAND
                    bash ${CMAKE_CURRENT_LIST_DIR}/utils/split_debug_info.sh "${file}"
                WORKING_DIRECTORY
                    "${CMAKE_INSTALL_PREFIX}/bin/"
            )
        endif()
    endforeach()
endif()

file(
    REMOVE_RECURSE
    "${DEBUG_PATH}"
)

file(
    REMOVE_RECURSE
    "${RELEASE_PATH}"
)

execute_process(
    COMMAND
        "${CMAKE_COMMAND}" -E tar cf "${PACKAGE_NAME}.7z" --format=7zip -- "${PACKAGE_NAME}"
    WORKING_DIRECTORY
        "${ROOT}"
)
