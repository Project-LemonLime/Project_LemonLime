# ==================================================================================
# Lemon Base
# ==================================================================================

set(LEMON_BASEDIR_BASE ${CMAKE_SOURCE_DIR}/src/base)

if(WIN32)
    include(CheckCXXSourceCompiles)
    include(CMakePushCheckState)
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_LIBRARIES ${LEMON_QT_LIBNAME}::Core)
    check_cxx_source_compiles([=[
        #include <qt_windows.h>
        int main() {
            PROCESSOR_RELATIONSHIP processor{};
            processor.EfficiencyClass = 0;
            DWORD size = 0;
            return GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &size);
        }
    ]=] LEMON_HAS_WINDOWS_CPU_TOPOLOGY)
    cmake_pop_check_state()
    if(NOT LEMON_HAS_WINDOWS_CPU_TOPOLOGY)
        message(FATAL_ERROR
            "CPU topology detection requires a Windows 10 or newer SDK exposing "
            "PROCESSOR_RELATIONSHIP::EfficiencyClass and GetLogicalProcessorInformationEx. "
            "Use the Qt Windows target definitions or _WIN32_WINNT >= 0x0601.")
    endif()
endif()

aux_source_directory(${LEMON_BASEDIR_BASE} LEMON_BASE_SOURCES)

add_library(lemon-base STATIC
    ${LEMON_BASE_SOURCES}
    ${SINGLEAPPLICATION_SOURCES}
    )

target_precompile_headers(lemon-base PUBLIC ${CMAKE_SOURCE_DIR}/src/pch.h)

target_link_libraries(lemon-base
    ${LEMON_QT_LIBS}
    ${SINGLEAPPLICATION_LIBRARY}
    )

target_include_directories(lemon-base PUBLIC
    ${CMAKE_BINARY_DIR}
    ${LEMON_BASEDIR_BASE}
    ${SINGLEAPPLICATION_DIR}
    ${spdlog_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
