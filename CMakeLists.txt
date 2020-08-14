cmake_minimum_required(VERSION 3.9.0 )

project(lemon)

option(EMBED_TRANSLATIONS "Embed translations" ON)

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


set(SRC
	src/main.cpp
	src/exttestcasemodifier.cpp
	src/exttestcasemodifierdialog.cpp
	src/exttestcasetable.cpp
	src/exttestcaseupdaterdialog.cpp
	src/visualsettings.cpp
	src/lemon.cpp
	src/contest.cpp
	src/task.cpp
	src/testcase.cpp
	src/settings.cpp
	src/compiler.cpp
	src/filelineedit.cpp
	src/summarytree.cpp
	src/taskeditwidget.cpp
	src/testcaseeditwidget.cpp
	src/generalsettings.cpp
	src/compilersettings.cpp
	src/addtestcaseswizard.cpp
	src/contestant.cpp
	src/judgingdialog.cpp
	src/judgingthread.cpp
	src/optionsdialog.cpp
	src/resultviewer.cpp
	src/assignmentthread.cpp
	src/detaildialog.cpp
	src/newcontestwidget.cpp
	src/opencontestwidget.cpp
	src/newcontestdialog.cpp
	src/opencontestdialog.cpp
	src/welcomedialog.cpp
	src/addtaskdialog.cpp
	src/advancedcompilersettingsdialog.cpp
	src/environmentvariablesdialog.cpp
	src/editvariabledialog.cpp
	src/addcompilerwizard.cpp
	src/exportutil.cpp
	src/statisticsbrowser.cpp
	)

set(HEADERS
	src/lemon.h
	src/exttestcasemodifier.h
	src/contest.h
	src/exttestcasemodifierdialog.h
	src/exttestcasetable.h
	src/exttestcaseupdaterdialog.h
	src/subtaskdependencelib.h
	src/visualsettings.h
	src/task.h
	src/testcase.h
	src/settings.h
	src/compiler.h
	src/filelineedit.h
	src/summarytree.h
	src/taskeditwidget.h
	src/testcaseeditwidget.h
	src/generalsettings.h
	src/compilersettings.h
	src/addtestcaseswizard.h
	src/contestant.h
	src/judgingdialog.h
	src/judgingthread.h
	src/optionsdialog.h
	src/resultviewer.h
	src/assignmentthread.h
	src/globaltype.h
	src/detaildialog.h
	src/newcontestwidget.h
	src/opencontestwidget.h
	src/newcontestdialog.h
	src/opencontestdialog.h
	src/welcomedialog.h
	src/addtaskdialog.h
	src/advancedcompilersettingsdialog.h
	src/environmentvariablesdialog.h
	src/editvariabledialog.h
	src/addcompilerwizard.h
	src/exportutil.h
	src/statisticsbrowser.h
	src/judgesharedvariables.h
	)

set(LEMON_UI_FORMS
	src/forms/lemon.ui
	src/forms/exttestcasemodifierdialog.ui
	src/forms/exttestcasemodifier.ui
	src/forms/exttestcaseupdaterdialog.ui
	src/forms/taskeditwidget.ui
	src/forms/testcaseeditwidget.ui
	src/forms/generalsettings.ui
	src/forms/compilersettings.ui
	src/forms/addtestcaseswizard.ui
	src/forms/judgingdialog.ui
	src/forms/optionsdialog.ui
	src/forms/detaildialog.ui
	src/forms/newcontestwidget.ui
	src/forms/opencontestwidget.ui
	src/forms/newcontestdialog.ui
	src/forms/opencontestdialog.ui
	src/forms/visualsettings.ui
	src/forms/welcomedialog.ui
	src/forms/addtaskdialog.ui
	src/forms/advancedcompilersettingsdialog.ui
	src/forms/environmentvariablesdialog.ui
	src/forms/editvariabledialog.ui
	src/forms/addcompilerwizard.ui
	src/forms/statisticsbrowser.ui
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

if(WIN32)
    target_link_libraries(lemon Qt5::Widgets Qt5::Gui Qt5::Network Qt5::Core Qt5::AxContainer SingleApplication::SingleApplication)
else()
    target_link_libraries(lemon Qt5::Widgets Qt5::Gui Qt5::Network Qt5::Core SingleApplication::SingleApplication)
endif()

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
