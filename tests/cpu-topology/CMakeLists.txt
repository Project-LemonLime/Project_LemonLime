add_executable(cpu-topology-test main.cpp)
target_link_libraries(cpu-topology-test PRIVATE
    lemon-base
    SingleApplication::SingleApplication
    ${LEMON_QT_LIBNAME}::Test
    spdlog::spdlog
)

add_test(NAME cpu-topology COMMAND cpu-topology-test)
set_tests_properties(cpu-topology PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    TIMEOUT 30
)
