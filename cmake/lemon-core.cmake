set(LEMON_BASEDIR_CORE ${CMAKE_SOURCE_DIR}/src/core)

set(LEMON_CORE_SOURCES
    ${LEMON_BASEDIR_CORE}/settings.h
    ${LEMON_BASEDIR_CORE}/settings.cpp
    ${LEMON_BASEDIR_CORE}/compiler.h
    ${LEMON_BASEDIR_CORE}/compiler.cpp
    ${LEMON_BASEDIR_CORE}/testcase.h
    ${LEMON_BASEDIR_CORE}/testcase.cpp
    ${LEMON_BASEDIR_CORE}/task.h
    ${LEMON_BASEDIR_CORE}/task.cpp
    ${LEMON_BASEDIR_CORE}/judgingthread.h
    ${LEMON_BASEDIR_CORE}/judgingthread.cpp
    ${LEMON_BASEDIR_CORE}/assignmentthread.h
    ${LEMON_BASEDIR_CORE}/assignmentthread.cpp
    ${LEMON_BASEDIR_CORE}/contestant.h
    ${LEMON_BASEDIR_CORE}/contestant.cpp
    ${LEMON_BASEDIR_CORE}/judgesharedvariables.h
    ${LEMON_BASEDIR_CORE}/subtaskdependencelib.h
) 
