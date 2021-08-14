# ==================================================================================
# Lemon Core
# ==================================================================================

set(LEMON_BASEDIR_CORE ${CMAKE_SOURCE_DIR}/src/core)

set(LEMON_CORE_SOURCES
    ${LEMON_BASEDIR_CORE}/contest.cpp
    ${LEMON_BASEDIR_CORE}/contest.h
    ${LEMON_BASEDIR_CORE}/testcase.h
    ${LEMON_BASEDIR_CORE}/testcase.cpp
    ${LEMON_BASEDIR_CORE}/task.h
    ${LEMON_BASEDIR_CORE}/task.cpp
    ${LEMON_BASEDIR_CORE}/judgingthread.h
    ${LEMON_BASEDIR_CORE}/judgingthread.cpp
    ${LEMON_BASEDIR_CORE}/contestant.h
    ${LEMON_BASEDIR_CORE}/contestant.cpp
    ${LEMON_BASEDIR_CORE}/judgesharedvariables.h
    ${LEMON_BASEDIR_CORE}/subtaskdependencelib.h
    ${LEMON_BASEDIR_CORE}/taskjudger.cpp
    ${LEMON_BASEDIR_CORE}/taskjudger.h
) 

add_library(lemon-core STATIC
    ${LEMON_CORE_SOURCES}
    ${LEMON_BASE_HEADERS}
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
