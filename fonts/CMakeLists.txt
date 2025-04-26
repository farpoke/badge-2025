
find_package(Python3 REQUIRED)

set(FONT_SOURCE_LIST "")
function(add_font NAME SIZE BPP PATH)
    set(FONT_HPP_PATH "${CMAKE_CURRENT_LIST_DIR}/cpp/${NAME}.hpp")
    set(FONT_CPP_PATH "${CMAKE_CURRENT_LIST_DIR}/cpp/${NAME}.cpp")
    add_custom_command(
        OUTPUT
            "${FONT_HPP_PATH}"
            "${FONT_CPP_PATH}"
        COMMAND
            ${Python3_EXECUTABLE} ttf.py ${NAME} ${SIZE} ${BPP} "${PATH}"
        DEPENDS
            "${CMAKE_CURRENT_LIST_DIR}/ttf.py"
            ${PATH}
        WORKING_DIRECTORY
            "${CMAKE_CURRENT_LIST_DIR}"
        COMMENT
            "Generate font ${NAME} ..."
        VERBATIM
    )
    set(FONT_SOURCE_LIST "${FONT_SOURCE_LIST}" "${FONT_HPP_PATH}" "${FONT_CPP_PATH}" PARENT_SCOPE)
endfunction()

add_font(m5x7           16 1 ttf/m5x7.ttf)
add_font(m6x11          16 1 ttf/m6x11.ttf)
add_font(noto_sans      12 4 ttf/Noto_Sans/static/NotoSans-Regular.ttf)
add_font(noto_sans_cm   12 4 ttf/Noto_Sans/static/NotoSans_Condensed-Medium.ttf)
add_font(lucida         12 4 C:/Windows/Fonts/L_10646.ttf)

add_library(fonts INTERFACE ${FONT_SOURCE_LIST})
target_sources(fonts INTERFACE ${FONT_SOURCE_LIST})
target_include_directories(fonts INTERFACE ${CMAKE_CURRENT_LIST_DIR}/cpp)
