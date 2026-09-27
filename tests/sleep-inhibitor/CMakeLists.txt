find_package(Qt${LEMON_QT_MAJOR_VERSION} ${LEMON_QT_MIN_VERSION} COMPONENTS Test REQUIRED)

add_executable(sleep-inhibitor-test main.cpp)
target_link_libraries(sleep-inhibitor-test PRIVATE
    lemon-core
    ${LEMON_QT_LIBNAME}::Test
    spdlog::spdlog
    )

add_test(NAME sleep-inhibitor COMMAND sleep-inhibitor-test -o sleep-inhibitor-results.txt,txt -o -,txt)
set_tests_properties(sleep-inhibitor PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} TIMEOUT 30)
