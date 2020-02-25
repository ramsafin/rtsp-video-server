# ==============================================
# Try to find FFmpeg libraries:
# - libavutil
# - libavcodec
# - libavformat
# - libavdevice
# - libavfilter
# - libswscale
# - libswresample
# - listpostproc
#
# FFmpeg_FOUND - FFmpeg libraries are found
# FFmpeg_INCLUDE_DIRS - FFmpeg include directories list
# FFmpeg_LIBRARIES - Link these to use FFmpeg
# ==============================================

# location of FFmpeg libs and include directory
set(FFmpeg_DIR ${CMAKE_SOURCE_DIR}/libs/ffmpeg)

# set PKG_CONFIG_PATH
set(FFMPEG_PKG_CONFIG_PATH "${FFmpeg_DIR}/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "${FFMPEG_PKG_CONFIG_PATH}")

set(FFmpeg_LIBRARIES "")
set(FFmpeg_INCLUDE_DIRS "")
set(FFmpeg_FOUND FALSE)

find_package(PkgConfig REQUIRED)

# add 'libva' to the list if you need hardware encoding capabilities
foreach(FFmpeg_LIBRARY libavcodec libavformat libavutil
        libavdevice libavfilter libswscale libswresample x264 x265)
    pkg_check_modules(FFmpeg_${FFmpeg_LIBRARY} REQUIRED ${FFmpeg_LIBRARY})
    list(APPEND FFmpeg_LIBRARIES ${FFmpeg_${FFmpeg_LIBRARY}_LDFLAGS})
endforeach()

if (EXISTS "${FFmpeg_DIR}/include")
    set(FFmpeg_INCLUDE_DIRS ${FFmpeg_DIR}/include)
else()
    message(FATAL_ERROR "Could not find FFmpeg include dirs.")
endif()

if (FFmpeg_LIBRARIES AND FFmpeg_INCLUDE_DIRS)
    set(FFmpeg_FOUND TRUE)
endif ()