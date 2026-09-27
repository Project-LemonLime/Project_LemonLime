# ==================================================================================
# Lemon Core
# ==================================================================================

set(LEMON_BASEDIR_CORE ${CMAKE_SOURCE_DIR}/src/core)

aux_source_directory(${LEMON_BASEDIR_CORE} LEMON_CORE_SOURCES)

add_library(lemon-core STATIC
    ${LEMON_CORE_SOURCES}
    ${SINGLEAPPLICATION_SOURCES}
    )

target_precompile_headers(lemon-core PUBLIC ${CMAKE_SOURCE_DIR}/src/pch.h)

target_link_libraries(lemon-core
    lemon-base
    ${LEMON_QT_LIBS}
    ${SINGLEAPPLICATION_LIBRARY}
    )

if(WIN32)
    target_link_libraries(lemon-core userenv advapi32 psapi)
elseif(APPLE)
    find_library(LEMON_IOKIT_FRAMEWORK IOKit)
    find_library(LEMON_COREFOUNDATION_FRAMEWORK CoreFoundation)
    target_link_libraries(lemon-core ${LEMON_IOKIT_FRAMEWORK} ${LEMON_COREFOUNDATION_FRAMEWORK})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(${LEMON_QT_LIBNAME} ${LEMON_QT_MIN_VERSION} COMPONENTS DBus REQUIRED)
    target_link_libraries(lemon-core ${LEMON_QT_LIBNAME}::DBus)
endif()

target_include_directories(lemon-core PUBLIC
    ${CMAKE_BINARY_DIR}
    ${LEMON_BASEDIR_CORE}
    ${SINGLEAPPLICATION_DIR}
    ${spdlog_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
