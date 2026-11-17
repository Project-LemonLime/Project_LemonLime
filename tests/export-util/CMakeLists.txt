add_executable(export-util-tests
    main.cpp
    ${CMAKE_SOURCE_DIR}/src/component/exportutil/exportutil.cpp
)
if(NOT WIN32)
    target_sources(export-util-tests PRIVATE ${CMAKE_BINARY_DIR}/watcher.qrc)
    add_dependencies(export-util-tests watcher_unix)
endif()

target_precompile_headers(export-util-tests REUSE_FROM lemon-core)
get_target_property(LEMON_CORE_IPO lemon-core INTERPROCEDURAL_OPTIMIZATION)
if(LEMON_CORE_IPO)
    set_property(TARGET export-util-tests PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_link_libraries(export-util-tests PRIVATE
    lemon-core
    SingleApplication::SingleApplication
    ${LEMON_QT_LIBNAME}::Test
    spdlog::spdlog
)
if(WIN32 AND ENABLE_XLS_EXPORT)
    target_link_libraries(export-util-tests PRIVATE ${LEMON_QT_LIBNAME}::AxContainer)
endif()

add_test(NAME export-util COMMAND export-util-tests)
set_tests_properties(export-util PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    TIMEOUT 30
)
