
find_package(Python3 REQUIRED)

set(IMAGE_PNG_PATH "${CMAKE_CURRENT_LIST_DIR}/../assets/gfx/promo-image.png")
set(IMAGE_HPP_PATH "${CMAKE_CURRENT_LIST_DIR}/image.hpp")
set(IMAGE_CPP_PATH "${CMAKE_CURRENT_BINARY_DIR}/gfx-data/image.cpp")

add_custom_command(
        OUTPUT
            "${IMAGE_HPP_PATH}"
            "${IMAGE_CPP_PATH}"
        COMMAND
            ${Python3_EXECUTABLE} rom-gen.py "${IMAGE_PNG_PATH}" "${IMAGE_CPP_PATH}"
        DEPENDS
            "${CMAKE_CURRENT_LIST_DIR}/rom-gen.py"
            "${IMAGE_PNG_PATH}"
        WORKING_DIRECTORY
            "${CMAKE_CURRENT_LIST_DIR}"
        COMMENT
            "Generate image data..."
        VERBATIM
)

add_library(gfx INTERFACE ${IMAGE_HPP_PATH} ${IMAGE_CPP_PATH})
target_sources(gfx INTERFACE ${IMAGE_HPP_PATH} ${IMAGE_CPP_PATH})
target_include_directories(gfx INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
