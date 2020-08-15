cmake_minimum_required(VERSION 3.9.0 )

project(lemon)

option(EMBED_TRANSLATIONS "Embed translations" ON)
option(ENABLE_XLS_EXPORT "XLS Result Export Support (Windows Only)" OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/src/forms)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)

if(WIN32)
    find_package(Qt5 COMPONENTS AxContainer REQUIRED)
    add_compile_options("/std:c++17")
    add_compile_definitions(UNICODE _UNICODE)
    set(GUI_TYPE WIN32)
endif()

# ==================================================================================
# Lemon Build Info
# ==================================================================================

file(STRINGS "${CMAKE_SOURCE_DIR}/makespec/VERSION" VERSION)
file(STRINGS "${CMAKE_SOURCE_DIR}/makespec/BUILDVERSION" BUILD_VERSION)
file(STRINGS "${CMAKE_SOURCE_DIR}/makespec/VERSIONSUFFIX" VERSION_SUFFIX)

set(VERSION_STRING "${VERSION}${VERSION_SUFFIX}")
set(VERSION_LIST ${VERSION})
string(REPLACE "." ";" VERSION_LIST ${VERSION_LIST})
separate_arguments(VERSION_LIST)

list(GET VERSION_LIST 0 CMAKE_PROJECT_VERSION_MAJOR)
list(GET VERSION_LIST 1 CMAKE_PROJECT_VERSION_MINOR)
list(GET VERSION_LIST 2 CPACK_PACKAGE_VERSION_PATCH)

add_definitions(-DLEMON_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR})
add_definitions(-DLEMON_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR})
add_definitions(-DLEMON_VERSION_BUGFIX=${CPACK_PACKAGE_VERSION_PATCH})
add_definitions(-DLEMON_VERSION_BUILD=${BUILD_VERSION})

add_definitions(-DLEMON_VERSION_STRING="${VERSION_STRING}")

message(" ")
message("LemonLime Version: ${VERSION_STRING}")
message("LemonLime Build Version: ${BUILD_VERSION}")
message("LemonLime Build Type: ${CMAKE_BUILD_TYPE}")
message("|-----------------------------------------------------|")
message("| LemonLime,                                          |")
message("| A tiny judging environment for OI contest.          |")
message("| Based on C++ & Qt.                                  |")
message("| Licenced under GPLv3                                |")
message("|                                                     |")
message("|-----------------------------------------------------|")
message("| GitHub: https://github.com/iotang/Project_LemonLime |")
message("| Welcome to contribute!                              |")
message("|-----------------------------------------------------|")
message(" ")

if(LEMON_BUILD_INFO)
    set(_LEMON_BUILD_INFO_STR_ "${LEMON_BUILD_INFO}")
elseif(DEFINED ENV{_LEMON_BUILD_INFO_})
    set(_LEMON_BUILD_INFO_STR_ "$ENV{_LEMON_BUILD_INFO_}")
else()
    set(_LEMON_BUILD_INFO_STR_ "LemonLime from manual build")
endif()

if(LEMON_BUILD_EXTRA_INFO)
    set(_LEMON_BUILD_EXTRA_INFO_STR_ "${LEMON_BUILD_EXTRA_INFO}")
elseif(DEFINED ENV{_LEMON_BUILD_EXTRA_INFO_})
    set(_LEMON_BUILD_EXTRA_INFO_STR_ "$ENV{_LEMON_BUILD_EXTRA_INFO_}")
else()
    set(_LEMON_BUILD_EXTRA_INFO_STR_ "LemonLime ${VERSION_STRING}:${BUILD_VERSION}")
endif()

set(LEMON_BUILD_INFO ${_LEMON_BUILD_INFO_STR_})
set(LEMON_BUILD_EXTRA_INFO ${_LEMON_BUILD_EXTRA_INFO_STR_})

add_definitions(-DLEMON_BUILD_INFO_STR="${_LEMON_BUILD_INFO_STR_}")
add_definitions(-DLEMON_BUILD_EXTRA_INFO_STR="${_LEMON_BUILD_EXTRA_INFO_STR_}")
message("LemonLime build info: ${_LEMON_BUILD_INFO_STR_}")
message("LemonLime build info extra: ${_LEMON_BUILD_EXTRA_INFO_STR_}")

if(LEMON_CONFIG_DIR)
    add_definitions(-DLEMON_CONFIG_DIR="${LEMON_CONFIG_DIR}")
endif()

set(LEMON_BASE_HEADERS
    ${CMAKE_SOURCE_DIR}/src/base/LemonBase.hpp
    ${CMAKE_SOURCE_DIR}/src/base/LemonLog.hpp
    )

set(SRC
	${CMAKE_SOURCE_DIR}/src/main.cpp
        ${CMAKE_SOURCE_DIR}/src/base/LemonLog.cpp
	${CMAKE_SOURCE_DIR}/src/exttestcasemodifier.cpp
	${CMAKE_SOURCE_DIR}/src/exttestcasemodifierdialog.cpp
	${CMAKE_SOURCE_DIR}/src/exttestcasetable.cpp
	${CMAKE_SOURCE_DIR}/src/exttestcaseupdaterdialog.cpp
	${CMAKE_SOURCE_DIR}/src/visualsettings.cpp
	${CMAKE_SOURCE_DIR}/src/lemon.cpp
	${CMAKE_SOURCE_DIR}/src/contest.cpp
	${CMAKE_SOURCE_DIR}/src/task.cpp
	${CMAKE_SOURCE_DIR}/src/testcase.cpp
	${CMAKE_SOURCE_DIR}/src/settings.cpp
	${CMAKE_SOURCE_DIR}/src/compiler.cpp
	${CMAKE_SOURCE_DIR}/src/filelineedit.cpp
	${CMAKE_SOURCE_DIR}/src/summarytree.cpp
	${CMAKE_SOURCE_DIR}/src/taskeditwidget.cpp
	${CMAKE_SOURCE_DIR}/src/testcaseeditwidget.cpp
	${CMAKE_SOURCE_DIR}/src/generalsettings.cpp
	${CMAKE_SOURCE_DIR}/src/compilersettings.cpp
	${CMAKE_SOURCE_DIR}/src/addtestcaseswizard.cpp
	${CMAKE_SOURCE_DIR}/src/contestant.cpp
	${CMAKE_SOURCE_DIR}/src/judgingdialog.cpp
	${CMAKE_SOURCE_DIR}/src/judgingthread.cpp
	${CMAKE_SOURCE_DIR}/src/optionsdialog.cpp
	${CMAKE_SOURCE_DIR}/src/resultviewer.cpp
	${CMAKE_SOURCE_DIR}/src/assignmentthread.cpp
	${CMAKE_SOURCE_DIR}/src/detaildialog.cpp
	${CMAKE_SOURCE_DIR}/src/newcontestwidget.cpp
	${CMAKE_SOURCE_DIR}/src/opencontestwidget.cpp
	${CMAKE_SOURCE_DIR}/src/newcontestdialog.cpp
	${CMAKE_SOURCE_DIR}/src/opencontestdialog.cpp
	${CMAKE_SOURCE_DIR}/src/welcomedialog.cpp
	${CMAKE_SOURCE_DIR}/src/addtaskdialog.cpp
	${CMAKE_SOURCE_DIR}/src/advancedcompilersettingsdialog.cpp
	${CMAKE_SOURCE_DIR}/src/environmentvariablesdialog.cpp
	${CMAKE_SOURCE_DIR}/src/editvariabledialog.cpp
	${CMAKE_SOURCE_DIR}/src/addcompilerwizard.cpp
	${CMAKE_SOURCE_DIR}/src/exportutil.cpp
	${CMAKE_SOURCE_DIR}/src/statisticsbrowser.cpp
	)

set(HEADERS
	${CMAKE_SOURCE_DIR}/src/lemon.h
	${CMAKE_SOURCE_DIR}/src/exttestcasemodifier.h
	${CMAKE_SOURCE_DIR}/src/contest.h
	${CMAKE_SOURCE_DIR}/src/exttestcasemodifierdialog.h
	${CMAKE_SOURCE_DIR}/src/exttestcasetable.h
	${CMAKE_SOURCE_DIR}/src/exttestcaseupdaterdialog.h
	${CMAKE_SOURCE_DIR}/src/subtaskdependencelib.h
	${CMAKE_SOURCE_DIR}/src/visualsettings.h
	${CMAKE_SOURCE_DIR}/src/task.h
	${CMAKE_SOURCE_DIR}/src/testcase.h
	${CMAKE_SOURCE_DIR}/src/settings.h
	${CMAKE_SOURCE_DIR}/src/compiler.h
	${CMAKE_SOURCE_DIR}/src/filelineedit.h
	${CMAKE_SOURCE_DIR}/src/summarytree.h
	${CMAKE_SOURCE_DIR}/src/taskeditwidget.h
	${CMAKE_SOURCE_DIR}/src/testcaseeditwidget.h
	${CMAKE_SOURCE_DIR}/src/generalsettings.h
	${CMAKE_SOURCE_DIR}/src/compilersettings.h
	${CMAKE_SOURCE_DIR}/src/addtestcaseswizard.h
	${CMAKE_SOURCE_DIR}/src/contestant.h
	${CMAKE_SOURCE_DIR}/src/judgingdialog.h
	${CMAKE_SOURCE_DIR}/src/judgingthread.h
	${CMAKE_SOURCE_DIR}/src/optionsdialog.h
	${CMAKE_SOURCE_DIR}/src/resultviewer.h
	${CMAKE_SOURCE_DIR}/src/assignmentthread.h
	${CMAKE_SOURCE_DIR}/src/globaltype.h
	${CMAKE_SOURCE_DIR}/src/detaildialog.h
	${CMAKE_SOURCE_DIR}/src/newcontestwidget.h
	${CMAKE_SOURCE_DIR}/src/opencontestwidget.h
	${CMAKE_SOURCE_DIR}/src/newcontestdialog.h
	${CMAKE_SOURCE_DIR}/src/opencontestdialog.h
	${CMAKE_SOURCE_DIR}/src/welcomedialog.h
	${CMAKE_SOURCE_DIR}/src/addtaskdialog.h
	${CMAKE_SOURCE_DIR}/src/advancedcompilersettingsdialog.h
	${CMAKE_SOURCE_DIR}/src/environmentvariablesdialog.h
	${CMAKE_SOURCE_DIR}/src/editvariabledialog.h
	${CMAKE_SOURCE_DIR}/src/addcompilerwizard.h
	${CMAKE_SOURCE_DIR}/src/exportutil.h
	${CMAKE_SOURCE_DIR}/src/statisticsbrowser.h
	${CMAKE_SOURCE_DIR}/src/judgesharedvariables.h
	)

set(LEMON_UI_FORMS
	${CMAKE_SOURCE_DIR}/src/forms/lemon.ui
	${CMAKE_SOURCE_DIR}/src/forms/exttestcasemodifierdialog.ui
	${CMAKE_SOURCE_DIR}/src/forms/exttestcasemodifier.ui
	${CMAKE_SOURCE_DIR}/src/forms/exttestcaseupdaterdialog.ui
	${CMAKE_SOURCE_DIR}/src/forms/taskeditwidget.ui
	${CMAKE_SOURCE_DIR}/src/forms/testcaseeditwidget.ui
	${CMAKE_SOURCE_DIR}/src/forms/generalsettings.ui
	${CMAKE_SOURCE_DIR}/src/forms/compilersettings.ui
	${CMAKE_SOURCE_DIR}/src/forms/addtestcaseswizard.ui
	${CMAKE_SOURCE_DIR}/src/forms/judgingdialog.ui
	${CMAKE_SOURCE_DIR}/src/forms/optionsdialog.ui
	${CMAKE_SOURCE_DIR}/src/forms/detaildialog.ui
	${CMAKE_SOURCE_DIR}/src/forms/newcontestwidget.ui
	${CMAKE_SOURCE_DIR}/src/forms/opencontestwidget.ui
	${CMAKE_SOURCE_DIR}/src/forms/newcontestdialog.ui
	${CMAKE_SOURCE_DIR}/src/forms/opencontestdialog.ui
	${CMAKE_SOURCE_DIR}/src/forms/visualsettings.ui
	${CMAKE_SOURCE_DIR}/src/forms/welcomedialog.ui
	${CMAKE_SOURCE_DIR}/src/forms/addtaskdialog.ui
	${CMAKE_SOURCE_DIR}/src/forms/advancedcompilersettingsdialog.ui
	${CMAKE_SOURCE_DIR}/src/forms/environmentvariablesdialog.ui
	${CMAKE_SOURCE_DIR}/src/forms/editvariabledialog.ui
	${CMAKE_SOURCE_DIR}/src/forms/addcompilerwizard.ui
	${CMAKE_SOURCE_DIR}/src/forms/statisticsbrowser.ui
	)

# ==================================================================================
# Lemon Translations
# ==================================================================================

include(cmake/translations.cmake)

if(EMBED_TRANSLATIONS)
    add_definitions(-DEMBED_TRANSLATIONS)
    configure_file(translations/translations.qrc ${CMAKE_BINARY_DIR} COPYONLY)
    set(LEMON_EMBED_TRANSLATION_QRC ${CMAKE_BINARY_DIR}/translations.qrc)
endif()

# Qt language translations
add_custom_target(lupdate
    COMMAND lupdate ${HEADERS}
    ${LEMON_BASE_HEADERS}
    ${SRC}
    ${LEMON_UI_FORMS}
    ${SINGLEAPPLICATION_SOURCES} -ts ${TRANSLATIONS_TS} -locations none -noobsolete
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_target_properties(lupdate PROPERTIES EXCLUDE_FROM_ALL TRUE)

set(QRC_RESOURCES 
    resource.qrc 
    manual/manual.qrc
    )
	
set(LEMON_FULL_SOURCES
    ${HEADERS}
    ${LEMON_BASE_HEADERS}
    ${SRC}
    assets/lemon.rc
    ${LEMON_UI_FORMS}
    ${LEMON_UI_RESOURCES}
    ${SINGLEAPPLICATION_SOURCES}
    ${LEMON_EMBED_TRANSLATION_QRC}
    ${QRC_RESOURCES}
    ${QM_FILES}
    )


# Set Apple MacOS Specific settings
if(APPLE)
    set(GUI_TYPE MACOSX_BUNDLE)
endif()

if(WIN32)
    add_executable(lemon ${GUI_TYPE} ${LEMON_FULL_SOURCES})
else()
    if(APPLE)
        add_executable(watcher_unix unix/watcher_macos.cpp)
    else()
        add_executable(watcher_unix unix/watcher_linux.cpp)
    endif()
    configure_file(unix/watcher.qrc ${CMAKE_BINARY_DIR} COPYONLY)
    add_executable(lemon ${GUI_TYPE} ${LEMON_FULL_SOURCES} ${CMAKE_BINARY_DIR}/watcher.qrc)
    add_dependencies(lemon watcher_unix)
endif()

target_include_directories(lemon PUBLIC ${CMAKE_SOURCE_DIR}/src)

set(SINGLEAPPLICATION_DIR ${CMAKE_SOURCE_DIR}/3rdparty/SingleApplication)
set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")
add_subdirectory(3rdparty/SingleApplication)

set(LEMON_LINK_LIB Qt5::Widgets Qt5::Gui Qt5::Network Qt5::Core SingleApplication::SingleApplication)

if(WIN32 AND ENABLE_XLS_EXPORT)
    set(LEMON_LINK_LIB ${LEMON_LINK_LIB} Qt5::AxContainer)
    add_definitions(-DENABLE_XLS_EXPORT)
endif()

target_link_libraries(lemon ${LEMON_LINK_LIB})

include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT error)

if(LTO_SUPPORTED)
    message(STATUS "IPO / LTO enabled")
    set_property(TARGET lemon PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO / LTO not supported, disabled")
endif()

# Need CMake 3.14
#cmake_policy(SET CMP0083 NEW)
#include(CheckPIESupported)
#check_pie_supported(OUTPUT_VARIABLE output LANGUAGES C)
#
#if(CMAKE_C_LINK_PIE_SUPPORTED)
#    message(STATUS "PIE enabled")
#    set_property(TARGET lemon PROPERTY POSITION_INDEPENDENT_CODE TRUE)
#else()
#    message(STATUS "PIE not supported, disabled")
#endif()

if(UNIX AND NOT APPLE AND NOT WIN32)
    install(TARGETS lemon RUNTIME DESTINATION bin)
    
    set(LEMON_LINUX_ICON_DIMENSIONS 16 22 24 32 36 44 48 64 72 96 128 150 192 256 310 512 1024)
    #install(FILES assets/lemon-lime.metainfo.xml DESTINATION share/metainfo)
    install(FILES assets/lemon-lime.desktop DESTINATION share/applications)
    #install(FILES assets/icons/lemon-lime.svg DESTINATION share/icons/hicolor/scalable/apps)
    foreach(LEMON_LINUX_ICON_DIMENSION ${LEMON_LINUX_ICON_DIMENSIONS})
        install(FILES assets/icons/lemon-lime.${LEMON_LINUX_ICON_DIMENSION}.png DESTINATION share/icons/hicolor/${LEMON_LINUX_ICON_DIMENSION}x${LEMON_LINUX_ICON_DIMENSION}/apps RENAME lemon-lime.png)
    endforeach(LEMON_LINUX_ICON_DIMENSION)
endif()

if(UNIX)
    if(Qt5Core_VERSION VERSION_LESS  5.4.0)
        execute_process(COMMAND sed -i '/<property name="tabBarAutoHide">/,+2d' ${CMAKE_SOURCE_DIR}/src/forms/lemon.ui)
    endif()
endif()


# ==================================================================================
# Lemon Package
# ==================================================================================

# Packaging
set(CPACK_PACKAGE_NAME "Lemon")
set(CPACK_PACKAGE_VENDOR "Coelacanthus")
set(CPACK_PACKAGE_VERSION ${VERSION_STRING})
set(CPACK_PACKAGE_DESCRIPTION "A tiny judging environment for OI contest based on Lemon + LemonPlus.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/iotang/Project_LemonLime")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/assets/lemon-lime.ico")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

if(UNIX)
    if(BUILD_DEB)
        set(CPACK_GENERATOR "DEB")
        set(CPACK_DEBIAN_PACKAGE_NAME "Lemon")
        set(CPACK_PACKAGE_CONTACT "liuhongwu2003@outlook.com")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Coelacanthus")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "qtbase5-dev, qttools5-dev")
    endif()

    #if(BUILD_RPM)
    #    set(CPACK_GENERATOR "RPM")
    #    set(CPACK_RPM_PACKAGE_LICENSE "GPL3")
    #endif()
endif()

# Must Here
include(CPack)
