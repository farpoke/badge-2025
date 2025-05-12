
find_package(Python3 REQUIRED)

execute_process(
        COMMAND ${Python3_EXECUTABLE} process-assets.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE ASSET_DEPENDENCIES_LINES
        COMMAND_ERROR_IS_FATAL ANY
)
string(REPLACE "\n" ";" ASSET_DEPENDENCIES "${ASSET_DEPENDENCIES_LINES}")

set(ASSETS_HPP "${CMAKE_CURRENT_BINARY_DIR}/assets.hpp")
set(ASSETS_CPP "${CMAKE_CURRENT_BINARY_DIR}/assets.cpp")

add_custom_command(
        OUTPUT "${ASSETS_HPP}" "${ASSETS_CPP}"
        COMMAND "${Python3_EXECUTABLE}" process-assets.py "${CMAKE_CURRENT_BINARY_DIR}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
        DEPENDS
            "${CMAKE_CURRENT_LIST_DIR}/process-assets.py"
            "${CMAKE_CURRENT_LIST_DIR}/assets/__init__.py"
            "${CMAKE_CURRENT_LIST_DIR}/assets/animation.py"
            "${CMAKE_CURRENT_LIST_DIR}/assets/base.py"
            "${CMAKE_CURRENT_LIST_DIR}/assets/filesystem.py"
            "${CMAKE_CURRENT_LIST_DIR}/assets/font.py"
            "${CMAKE_CURRENT_LIST_DIR}/assets/image.py"
            "${CMAKE_CURRENT_LIST_DIR}/assets.yaml"
            "${ASSET_DEPENDENCIES}"
        COMMENT "Generate asset data..."
        VERBATIM
)

add_library(assets INTERFACE "${ASSETS_HPP}" "${ASSETS_CPP}")
target_sources(assets INTERFACE "${ASSETS_HPP}" "${ASSETS_CPP}")
target_include_directories(assets INTERFACE "${CMAKE_CURRENT_BINARY_DIR}")
