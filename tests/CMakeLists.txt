find_package(Qt${LEMON_QT_MAJOR_VERSION} ${LEMON_QT_MIN_VERSION} COMPONENTS Test REQUIRED)

add_subdirectory(integrated-test)
if(WIN32)
    add_subdirectory(windows-sandbox)
    add_subdirectory(sleep-inhibitor)
endif()
