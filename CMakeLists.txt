cmake_minimum_required(VERSION 3.5)

project(video_server)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(PkgConfig)
pkg_check_modules(LOG4CPP REQUIRED log4cpp)

find_package(Live555 REQUIRED)
find_package(FFmpeg REQUIRED)

include_directories(${FFmpeg_INCLUDE_DIRS})
include_directories(${Live555_INCLUDE_DIRS})
include_directories(${YAML_CPP_INCLUDE_DIR})

include_directories("include")
file(GLOB SOURCE_FILES "src/*.cpp" "src/config/*.cpp")

add_subdirectory(contrib)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(
        ${PROJECT_NAME}
        ${LOG4CPP_LIBRARIES}
        ${Live555_LIBRARIES}
        yaml-cpp
        ${FFmpeg_LIBRARIES}
)
