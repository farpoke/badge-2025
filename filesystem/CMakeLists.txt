
find_package(Python3 REQUIRED)

set(MSC_CPP_PATH "${CMAKE_CURRENT_LIST_DIR}/msc.cpp")
set(DISK_CPP_PATH "${CMAKE_CURRENT_LIST_DIR}/disk.cpp")

set(PDF_ARCHIVE_PATH "${CMAKE_CURRENT_LIST_DIR}/files/badge-pdfs.tar.gz")
set(PDF_ARCHIVE_CONTENT
        badge-drawings.pdf
        badge-schematic.pdf
)
list(TRANSFORM PDF_ARCHIVE_CONTENT PREPEND ${CMAKE_CURRENT_LIST_DIR}/files/ OUTPUT_VARIABLE PDF_ARCHIVE_DEPENDENCIES)
message(STATUS ${PDF_ARCHIVE_DEPENDENCIES})

set(DISK_DEPENDENCIES
        ${CMAKE_CURRENT_LIST_DIR}/files/README.txt
        ${CMAKE_CURRENT_LIST_DIR}/files/hidden.txt
        ${PDF_ARCHIVE_PATH}
)

add_custom_command(
        OUTPUT ${PDF_ARCHIVE_PATH}
        COMMAND tar -zf ${PDF_ARCHIVE_PATH} -c ${PDF_ARCHIVE_CONTENT}
        DEPENDS ${PDF_ARCHIVE_DEPENDENCIES}
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/files"
        COMMENT "Create PDF archive..."
        VERBATIM
)

add_custom_command(
        OUTPUT "${DISK_CPP_PATH}"
        COMMAND "${Python3_EXECUTABLE}" gen-fs.py ${DISK_DEPENDENCIES}
        DEPENDS "${DISK_DEPENDENCIES}" "${CMAKE_CURRENT_LIST_DIR}/gen-fs.py"
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
        COMMENT "Generate disk image..."
        VERBATIM
)

add_library(filesystem INTERFACE ${MSC_CPP_PATH} ${DISK_CPP_PATH})
target_sources(filesystem INTERFACE ${MSC_CPP_PATH} ${DISK_CPP_PATH})
