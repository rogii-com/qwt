cmake_minimum_required(VERSION 3.26)

project(qwt CXX)

set(TARGET_NAME ${PROJECT_NAME})

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

set(CMAKE_DEBUG_POSTFIX d)

# prepare CNPM packages
set(
    ROGII_FOLDER_PATH
    "${CMAKE_CURRENT_LIST_DIR}/rogii"
)
include(${ROGII_FOLDER_PATH}/npm/Include.cmake)
include(${ROGII_FOLDER_PATH}/environment.cmake)

CNPM_PREPARE_PACKAGES(
    DEFAULT_REPOSITORY_URLS
    "$ENV{CNPM_URLS}"
)

file(GLOB_RECURSE srcs_cpp src/*.cpp)
file(GLOB_RECURSE hdrs src/*.h*)

add_definitions(-DDLL_EXPORTS)
add_definitions(${QT_DEFINITIONS})

add_library(${PROJECT_NAME} SHARED ${srcs_cpp}  ${hdrs})

target_include_directories(
    ${TARGET_NAME}
    PRIVATE
    src
    ${QT_USE_FILE}
)

target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Svg
    Qt5::PrintSupport
    Qt5::Concurrent
)

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS /SUBSYSTEM:WINDOWS)
endif (WIN32)

# libqwt generates a lot of warnings.
# We decided to supress it and make more visible warnings from gridding library
if(MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE /wd4996 /wd4267 /wd4005)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${TARGET_NAME} PRIVATE -Wno-deprecated-declarations)
endif()


install (TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION bin
)
install(FILES ${hdrs} DESTINATION include)

if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION bin OPTIONAL)
endif()