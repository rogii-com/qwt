include(${CMAKE_CURRENT_LIST_DIR}/msvs_package.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/windowssdk_package.cmake)

CNPM_ADD_PACKAGE(
    NAME
        Qt
    VERSION
        5.15.9
    BUILD_NUMBER
        10
    TAG
        "sdk20348_vsbt22"
)