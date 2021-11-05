# ==================================================================================
# Lemon Core
# ==================================================================================

set(LEMON_BASEDIR_CORE ${CMAKE_SOURCE_DIR}/src/core)

aux_source_directory(${LEMON_BASEDIR_CORE} LEMON_CORE_SOURCES)

add_library(lemon-core STATIC
    ${LEMON_CORE_SOURCES}
    ${SINGLEAPPLICATION_SOURCES}
    )

target_link_libraries(lemon-core
    lemon-base
    ${LEMON_QT_LIBS}
    ${SINGLEAPPLICATION_LIBRARY}
    )

target_include_directories(lemon-core PUBLIC
    ${CMAKE_BINARY_DIR}
    ${LEMON_BASEDIR_CORE}
    ${SINGLEAPPLICATION_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
