
find_package(Python3 REQUIRED)

set(IMAGE_HPP_PATH "${CMAKE_CURRENT_LIST_DIR}/image.hpp")
set(IMAGE_CPP_PATH "${CMAKE_CURRENT_LIST_DIR}/image.cpp")

add_custom_command(
        OUTPUT
            "${IMAGE_HPP_PATH}"
            "${IMAGE_CPP_PATH}"
        COMMAND
            ${Python3_EXECUTABLE} rom-gen.py
        DEPENDS
            "${CMAKE_CURRENT_LIST_DIR}/rom-gen.py"
        WORKING_DIRECTORY
            "${CMAKE_CURRENT_LIST_DIR}"
        COMMENT
            "Generate image data..."
        VERBATIM
)

add_library(gfx INTERFACE ${IMAGE_HPP_PATH} ${IMAGE_CPP_PATH})
target_sources(gfx INTERFACE ${IMAGE_HPP_PATH} ${IMAGE_CPP_PATH})
