
target_sources(${MICROPY_TARGET} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/drawing.cpp
        ${CMAKE_CURRENT_LIST_DIR}/font.cpp
        ${CMAKE_CURRENT_LIST_DIR}/lcd.cpp
        ${CMAKE_CURRENT_LIST_DIR}/modlcd.c
)

list(APPEND MICROPY_SOURCE_QSTR
        ${CMAKE_CURRENT_LIST_DIR}/modlcd.c
)
