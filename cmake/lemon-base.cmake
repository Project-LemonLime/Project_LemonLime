# ==================================================================================
# Lemon Base
# ==================================================================================

set(LEMON_BASEDIR_BASE ${CMAKE_SOURCE_DIR}/src/base)

aux_source_directory(${LEMON_BASEDIR_BASE} LEMON_BASE_SOURCES)

add_library(lemon-base STATIC
    ${LEMON_BASE_SOURCES}
    ${SINGLEAPPLICATION_SOURCES}
    )

target_precompile_headers(lemon-base PUBLIC ${CMAKE_SOURCE_DIR}/src/pch.h)

target_link_libraries(lemon-base
    ${LEMON_QT_LIBS}
    ${SINGLEAPPLICATION_LIBRARY}
    )

target_include_directories(lemon-base PUBLIC
    ${CMAKE_BINARY_DIR}
    ${LEMON_BASEDIR_BASE}
    ${SINGLEAPPLICATION_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
