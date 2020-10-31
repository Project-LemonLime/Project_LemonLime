# ==================================================================================
# Lemon Base
# ==================================================================================

set(LEMON_BASEDIR_BASE ${CMAKE_SOURCE_DIR}/src/base)

set(LEMON_BASE_HEADERS
    ${LEMON_BASEDIR_BASE}/LemonBase.hpp
    ${LEMON_BASEDIR_BASE}/LemonLog.hpp
    ${LEMON_BASEDIR_BASE}/LemonType.hpp
    ${LEMON_BASEDIR_BASE}/LemonConfig.hpp
    ${LEMON_BASEDIR_BASE}/LemonTranslator.hpp
    ${LEMON_BASEDIR_BASE}/LemonUtils.hpp
    ${LEMON_BASEDIR_BASE}/LemonBaseApplication.hpp
    )

set(LEMON_BASE_SOURCES
    ${LEMON_BASEDIR_BASE}/LemonConfig.cpp
    ${LEMON_BASEDIR_BASE}/LemonTranslator.cpp
    ${LEMON_BASEDIR_BASE}/LemonUtils.cpp
    ${LEMON_BASEDIR_BASE}/LemonBaseApplication.cpp
    )

add_library(lemon-base STATIC
    ${LEMON_BASE_SOURCES}
    ${LEMON_BASE_HEADERS}
    ${SINGLEAPPLICATION_SOURCES}
    )

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
