cmake_minimum_required(VERSION 3.0)
project(RTSPServer)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

find_package(FFmpeg REQUIRED)
find_package(Live555 REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)

include_directories("inc")

add_executable(
        ${PROJECT_NAME}
        ${SOURCE_FILES}
        src/main.cpp
        src/Utils.cpp
        src/LiveCamFramedSource.cpp
        src/Transcoder.cpp
        src/CameraUnicastServerMediaSubsession.cpp
        src/LiveCameraRTSPServer.cpp
)

# Boost
include_directories(${Boost_INCLUDE_DIR}/boost)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

# FFmpeg
if (FFMPEG_FOUND)
    include_directories(${FFMPEG_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBRARIES})
else (FFMPEG_FOUND)
    message(FATAL_ERROR "Can't find FFmpeg libs!")
endif (FFMPEG_FOUND)

# log4cpp
find_path(LOG4CPP_INCLUDE_DIR log4cpp/Category.hh)
if (LOG4CPP_INCLUDE_DIR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_LOG4CPP")
    include_directories(${LOG4CPP_INCLUDE_DIR})
    find_library(LOG4CPP_LIBRARY log4cpp)
    target_link_libraries(${PROJECT_NAME} ${LOG4CPP_LIBRARY})
else (LOG4CPP_INCLUDE_DIR)
    message(FATAL_ERROR "Can't find FFmpeg libs!")
endif (LOG4CPP_INCLUDE_DIR)

# Live555 Framework
if (Live555_FOUND)
    message("Found Live555")
    foreach (Live555_module IN LISTS Live555_INCLUDE_DIRS)
        include_directories(${Live555_module})
    endforeach ()
    target_link_libraries(${PROJECT_NAME} ${Live555_LIBRARIES})
else (Live555_FOUND)
    message(FATAL_ERROR "Can't find Live555 libraries")
endif (Live555_FOUND)
