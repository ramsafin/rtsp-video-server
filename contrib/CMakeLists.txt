# yaml-cpp
if (NOT EXISTS "${PROJECT_SOURCE_DIR}/contrib/yaml-cpp/CMakeLists.txt")
    message(FATAL_ERROR "submodule contrib/yaml-cpp is missing.
         To fix try run: \n git submodule update --init --recursive")
else ()
    # Disable testing and parse tools
    option(YAML_CPP_BUILD_TESTS "Enable testing" OFF)
    option(YAML_CPP_BUILD_TOOLS "Enable parse tools" OFF)
    add_subdirectory(${PROJECT_SOURCE_DIR}/contrib/yaml-cpp ${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp)
endif ()
