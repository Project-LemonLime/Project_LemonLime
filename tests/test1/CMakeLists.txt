# ==================================================================================
# Test 1: Contest Core Logic Integration Test
# ==================================================================================

find_package(Qt${LEMON_QT_MAJOR_VERSION} ${LEMON_QT_MIN_VERSION} COMPONENTS Test REQUIRED)

# Copy test data to build directory so the test can find it via a relative path
file(COPY ${CMAKE_SOURCE_DIR}/tests/data/TestContest1
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data)

set(TEST1_SOURCES
    main.cpp
)

if (NOT WIN32)
    list(APPEND TEST1_SOURCES ${CMAKE_BINARY_DIR}/watcher.qrc)
    add_executable(test1 ${TEST1_SOURCES})
    add_dependencies(test1 watcher_unix)
else()
    add_executable(test1 ${TEST1_SOURCES})
endif()


target_precompile_headers(test1 REUSE_FROM lemon-core)

target_include_directories(test1 PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
    ${spdlog_DIR}/include
)

target_link_libraries(test1 PRIVATE
    lemon-core
    lemon-base
    SingleApplication::SingleApplication
    ${LEMON_QT_LIBNAME}::Test
    ${LEMON_QT_LIBS}
    spdlog::spdlog
)

target_compile_definitions(test1 PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_BINARY_DIR}/data"
)

add_test(NAME test1 COMMAND test1)
set_tests_properties(test1 PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
