add_executable(judging-controller-tests main.cpp)
if(NOT WIN32)
    target_sources(judging-controller-tests PRIVATE ${CMAKE_BINARY_DIR}/watcher.qrc)
    add_dependencies(judging-controller-tests watcher_unix)
endif()

target_precompile_headers(judging-controller-tests REUSE_FROM lemon-core)
get_target_property(LEMON_CORE_IPO lemon-core INTERPROCEDURAL_OPTIMIZATION)
if(LEMON_CORE_IPO)
    set_property(TARGET judging-controller-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
target_link_libraries(judging-controller-tests PRIVATE
    lemon-core
    SingleApplication::SingleApplication
    ${LEMON_QT_LIBNAME}::Test
    spdlog::spdlog
)
add_test(NAME judging-controller COMMAND judging-controller-tests -o controller-results.txt,txt -o -,txt)
set_tests_properties(judging-controller PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    TIMEOUT 45
)
