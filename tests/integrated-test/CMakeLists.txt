add_executable(integrated-test main.cpp)
if(NOT WIN32)
    target_sources(integrated-test PRIVATE ${CMAKE_BINARY_DIR}/watcher.qrc)
    add_dependencies(integrated-test watcher_unix)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/enable-asan.cmake)

target_precompile_headers(integrated-test REUSE_FROM lemon-core)
target_link_libraries(integrated-test PRIVATE
    lemon-core
    SingleApplication::SingleApplication
    ${LEMON_QT_LIBNAME}::Test
    spdlog::spdlog
)
target_compile_definitions(integrated-test PRIVATE
    TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/tests/data/TestContest1"
)

add_test(NAME integrated-test COMMAND integrated-test)
set_tests_properties(integrated-test PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    TIMEOUT 120
)
