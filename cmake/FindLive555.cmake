# ==============================================
# Try to find Live555 libraries:
# - liveMedia
# - groupsock
# - BasicUsageEnvironment
# - UsageEnvironment
#
# Live555_FOUND - Live555 is found
# Live555_INCLUDE_DIRS - Live555 include directories list
# Live555_LIBRARIES - Link these to use Live555
#
# ==============================================

# location of Live555 sources
set(Live555_SOURCES ${CMAKE_SOURCE_DIR}/contrib/live)

set(Live555_LIBRARIES "")
set(Live555_INCLUDE_DIRS "")
set(Live555_FOUND FALSE)

# LIBS
foreach (Live555_module liveMedia groupsock BasicUsageEnvironment UsageEnvironment)
    find_library(
            ${Live555_module}_LIB ${Live555_module}
            PATHS ${Live555_SOURCES}/${Live555_module}
    )
    if (${Live555_module}_LIB)
        list(APPEND Live555_LIBRARIES ${${Live555_module}_LIB})
    elseif (NOT ${Live555_module}_LIB)
        set(Live555_FOUND FALSE)
        break()
    endif ()
endforeach (Live555_module)

# INCLUDES
foreach (Live555_module UsageEnvironment groupsock liveMedia BasicUsageEnvironment)
    if (EXISTS ${Live555_SOURCES}/${Live555_module})
        list(APPEND Live555_INCLUDE_DIRS ${Live555_SOURCES}/${Live555_module}/include)
    else (EXISTS ${Live555_SOURCES}/${Live555_module})
        set(Live555_FOUND FALSE)
        message(FATAL_ERROR "Could not find Live555 libraries!")
    endif (EXISTS ${Live555_SOURCES}/${Live555_module})
endforeach ()

if (Live555_INCLUDE_DIRS AND Live555_LIBRARIES)
    set(Live555_FOUND TRUE)
endif (Live555_INCLUDE_DIRS AND Live555_LIBRARIES)