# ==============================================
# Try to find FFmpeg libraries:
# - avcodec
# - avformat
# - avdevice
# - avutil
# - swscale
# - avfilter
#
# FFMPEG_FOUND - system has FFmpeg
# FFMPEG_INCLUDE_DIR - the FFmpeg inc directory
# FFMPEG_LIBRARIES - Link these to use FFmpeg
# ==============================================

if (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)
    # in cache already
    set(FFMPEG_FOUND TRUE)
else (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)

    find_path(
            FFMPEG_AVCODEC_INCLUDE_DIR
            NAMES libavcodec/avcodec.h
            PATHS ${_FFMPEG_AVCODEC_INCLUDE_DIRS}
            /usr/include
            /usr/local/include
            /opt/local/include
            PATH_SUFFIXES ffmpeg
    )

    find_library(
            FFMPEG_LIBAVCODEC
            NAMES avcodec
            PATHS ${_FFMPEG_AVCODEC_LIBRARY_DIRS}
            /usr/lib
            /usr/local/lib
            /opt/local/lib
    )

    find_library(
            FFMPEG_LIBAVFORMAT
            NAMES avformat
            PATHS ${_FFMPEG_AVFORMAT_LIBRARY_DIRS}
            /usr/lib
            /usr/local/lib
            /opt/local/lib
    )

    find_library(
            FFMPEG_LIBAVDEVICE
            NAMES avdevice
            PATHS ${_FFMPEG_AVDEVICE_LIBRARY_DIRS}
            /usr/lib
            /usr/local/lib
            /opt/local/lib
    )

    find_library(
            FFMPEG_LIBAVUTIL
            NAMES avutil
            PATHS ${_FFMPEG_AVUTIL_LIBRARY_DIRS}
            /usr/lib
            /usr/local/lib
            /opt/local/lib
    )

    find_library(
            FFMPEG_LIBSWSCALE
            NAMES swscale
            PATHS ${_FFMPEG_SWSCALE_LIBRARY_DIRS}
            /usr/lib
            /usr/local/lib
            /opt/local/lib
    )

    find_library(
            FFMPEG_LIBAVFILTER
            NAMES avfilter
            PATHS ${_FFMPEG_SWSCALE_LIBRARY_DIRS}
            /usr/lib
            /usr/local/lib
            /opt/local/lib
    )


    if (FFMPEG_LIBAVCODEC AND FFMPEG_LIBAVFORMAT AND FFMPEG_LIBSWSCALE AND FFMPEG_LIBAVDEVICE AND FFMPEG_LIBAVFILTER)
        set(FFMPEG_FOUND TRUE)
    endif ()

    if (FFMPEG_FOUND)
        set(FFMPEG_INCLUDE_DIR ${FFMPEG_AVCODEC_INCLUDE_DIR})
        set(FFMPEG_LIBRARIES
                ${FFMPEG_LIBAVCODEC}
                ${FFMPEG_LIBAVFORMAT}
                ${FFMPEG_LIBAVUTIL}
                ${FFMPEG_LIBSWSCALE}
                ${FFMPEG_LIBAVDEVICE}
                ${FFMPEG_LIBAVFILTER})
    else (FFMPEG_FOUND)
        message(FATAL_ERROR "Could not find FFmpeg libraries!")
    endif (FFMPEG_FOUND)

endif (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)