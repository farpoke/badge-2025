
find_package(Python3 REQUIRED)

set(MSC_CPP_PATH "${CMAKE_CURRENT_LIST_DIR}/msc.cpp")
set(DISK_CPP_PATH "${CMAKE_CURRENT_LIST_DIR}/disk.cpp")

set(DISK_DEPENDENCIES
        "${CMAKE_CURRENT_LIST_DIR}/files/README.txt"
        "${CMAKE_CURRENT_LIST_DIR}/files/badge-schematic.zip"
        "${CMAKE_CURRENT_LIST_DIR}/files/hidden.txt"
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
