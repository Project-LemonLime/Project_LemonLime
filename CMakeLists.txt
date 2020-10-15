cmake_minimum_required(VERSION 3.9.0 )

project(lemon)

macro(LEMONLOG MSG)
    set(LEMON_BUILD_STATS "${LEMON_BUILD_STATS}${MSG}: ${${MSG}}\r\n")
endmacro()

option(EMBED_TRANSLATIONS "Embed translations" ON)
LEMONLOG(EMBED_TRANSLATIONS)
option(ENABLE_XLS_EXPORT "XLS Result Export Support (Windows Only)" OFF)
LEMONLOG(ENABLE_XLS_EXPORT)
option(BUILD_DEB "Build deb format package" OFF)
LEMONLOG(BUILD_DEB)
option(BUILD_RPM "Build rpm format package" OFF)
LEMONLOG(BUILD_RPM)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/src/forms)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LEMON_QT6 "Use Qt6 for Debugging" OFF)
LEMONLOG(LEMON_QT6)
if(LEMON_QT6)
    cmake_policy(SET CMP0072 NEW)
    set(LEMON_QT_MAJOR_VERSION 6)
    set(LEMON_QT_MINOR_VERSION 0)
    set(LEMON_QT_LIBNAME Qt6)
    set(QT_MAJOR_VERSION 6)
    add_definitions(-DLEMON_QT6=1)
    #From: https://github.com/KDE/kwin/blob/master/cmake/modules/FindXKB.cmake#L35
    find_package(PkgConfig)
    pkg_check_modules(PKG_XKB QUIET xkbcommon)
    set(XKB_DEFINITIONS ${PKG_XKB_CFLAGS_OTHER})
    find_path(XKB_INCLUDE_DIR NAMES xkbcommon/xkbcommon.h HINTS ${PKG_XKB_INCLUDE_DIRS})
    find_library(XKB_LIBRARY NAMES xkbcommon HINTS ${PKG_XKB_LIBRARY_DIRS})
    add_library(XKB::XKB UNKNOWN IMPORTED)
    set_target_properties(XKB::XKB PROPERTIES
        IMPORTED_LOCATION "${XKB_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${XKB_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${XKB_INCLUDE_DIR}"
        )
else()
    set(LEMON_QT_MAJOR_VERSION 5)
    set(LEMON_QT_MINOR_VERSION 5)
    set(LEMON_QT_LIBNAME Qt5)
    set(QT_MAJOR_VERSION 5)
endif()

find_package(${LEMON_QT_LIBNAME} ${LEMON_QT_MAJOR_VERSION}.${LEMON_QT_MINOR_VERSION} COMPONENTS Core Network Core Gui Widgets Network REQUIRED)
list(APPEND LEMON_QT_LIBS ${LEMON_QT_LIBNAME}::Core ${LEMON_QT_LIBNAME}::Gui ${LEMON_QT_LIBNAME}::Widgets ${LEMON_QT_LIBNAME}::Network)

if(WIN32)
    add_compile_options("/std:c++17")
    add_compile_definitions(UNICODE _UNICODE)
    set(GUI_TYPE WIN32)
endif()

if(UNIX AND NOT APPLE)
	find_program(LSB_RELEASE_EXEC lsb_release)
	execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
		OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()
LEMONLOG(LSB_RELEASE_ID_SHORT)

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
message("|----------------------------------------------------------------|")
message("| LemonLime,                                                     |")
message("| A tiny judging environment for OI contest.                     |")
message("| Based on C++ & Qt.                                             |")
message("| Licenced under GPLv3                                           |")
message("|                                                                |")
message("|----------------------------------------------------------------|")
message("| GitHub: https://github.com/Project-LemonLime/Project_LemonLime |")
message("| Welcome to contribute!                                         |")
message("|----------------------------------------------------------------|")
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
    LEMONLOG(LEMON_CONFIG_DIR)
endif()

set(LEMON_BASE_HEADERS
    ${CMAKE_SOURCE_DIR}/src/base/LemonBase.hpp
    ${CMAKE_SOURCE_DIR}/src/base/LemonLog.hpp
    ${CMAKE_SOURCE_DIR}/src/base/LemonType.hpp
    ${CMAKE_SOURCE_DIR}/src/base/LemonConfig.hpp
    ${CMAKE_SOURCE_DIR}/src/base/LemonTranslator.hpp
    ${CMAKE_SOURCE_DIR}/src/base/LemonUtils.hpp
    )

set(LEMON_BASE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/base/LemonConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/base/LemonTranslator.cpp
    ${CMAKE_SOURCE_DIR}/src/base/LemonUtils.cpp
    )
    
set(SRC
    ${LEMON_BASE_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/exttestcasemodifier.cpp
    ${CMAKE_SOURCE_DIR}/src/exttestcasemodifierdialog.cpp
    ${CMAKE_SOURCE_DIR}/src/exttestcasetable.cpp
    ${CMAKE_SOURCE_DIR}/src/exttestcaseupdaterdialog.cpp
    ${CMAKE_SOURCE_DIR}/src/visualsettings.cpp
    ${CMAKE_SOURCE_DIR}/src/lemon.cpp
    ${CMAKE_SOURCE_DIR}/src/contest.cpp
    ${CMAKE_SOURCE_DIR}/src/filelineedit.cpp
    ${CMAKE_SOURCE_DIR}/src/summarytree.cpp
    ${CMAKE_SOURCE_DIR}/src/taskeditwidget.cpp
    ${CMAKE_SOURCE_DIR}/src/testcaseeditwidget.cpp
    ${CMAKE_SOURCE_DIR}/src/generalsettings.cpp
    ${CMAKE_SOURCE_DIR}/src/compilersettings.cpp
    ${CMAKE_SOURCE_DIR}/src/addtestcaseswizard.cpp
    ${CMAKE_SOURCE_DIR}/src/judgingdialog.cpp
    ${CMAKE_SOURCE_DIR}/src/optionsdialog.cpp
    ${CMAKE_SOURCE_DIR}/src/resultviewer.cpp
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
    ${CMAKE_SOURCE_DIR}/src/statisticsbrowser.cpp
    ${CMAKE_SOURCE_DIR}/src/component/exportutil/exportutil.cpp
    )

set(HEADERS
    ${CMAKE_SOURCE_DIR}/src/lemon.h
    ${CMAKE_SOURCE_DIR}/src/exttestcasemodifier.h
    ${CMAKE_SOURCE_DIR}/src/contest.h
    ${CMAKE_SOURCE_DIR}/src/exttestcasemodifierdialog.h
    ${CMAKE_SOURCE_DIR}/src/exttestcasetable.h
    ${CMAKE_SOURCE_DIR}/src/exttestcaseupdaterdialog.h
    ${CMAKE_SOURCE_DIR}/src/visualsettings.h
    ${CMAKE_SOURCE_DIR}/src/filelineedit.h
    ${CMAKE_SOURCE_DIR}/src/summarytree.h
    ${CMAKE_SOURCE_DIR}/src/taskeditwidget.h
    ${CMAKE_SOURCE_DIR}/src/testcaseeditwidget.h
    ${CMAKE_SOURCE_DIR}/src/generalsettings.h
    ${CMAKE_SOURCE_DIR}/src/compilersettings.h
    ${CMAKE_SOURCE_DIR}/src/addtestcaseswizard.h
    ${CMAKE_SOURCE_DIR}/src/judgingdialog.h
    ${CMAKE_SOURCE_DIR}/src/optionsdialog.h
    ${CMAKE_SOURCE_DIR}/src/resultviewer.h
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
    ${CMAKE_SOURCE_DIR}/src/statisticsbrowser.h
    ${CMAKE_SOURCE_DIR}/src/component/exportutil/exportutil.h
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
# Lemon Core
# ==================================================================================

include(cmake/lemon-core.cmake)

add_library(lemon-core STATIC
    ${LEMON_CORE_SOURCES}
    ${LEMON_BASE_HEADERS}
    ${SINGLEAPPLICATION_SOURCES}
    )

target_link_libraries(lemon-core
    ${LEMON_QT_LIBS}
    ${SINGLEAPPLICATION_LIBRARY}
    )

target_include_directories(lemon-core PUBLIC
    ${CMAKE_BINARY_DIR}
    ${QV2RAY_BASEDIR_CORE}
    ${SINGLEAPPLICATION_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
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
    ${LEMON_CORE_SOURCES}
    ${SRC}
    ${LEMON_UI_FORMS}
    ${SINGLEAPPLICATION_SOURCES} -ts ${TRANSLATIONS_TS} -locations none -noobsolete
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_target_properties(lupdate PROPERTIES EXCLUDE_FROM_ALL TRUE)


# ==================================================================================
# Lemon Build
# ==================================================================================

set(QRC_RESOURCES
    resource.qrc
    manual/manual.qrc
    )

set(LEMON_FULL_SOURCES
    ${HEADERS}
    ${LEMON_BASE_HEADERS}
    ${LEMON_CORE_SOURCES}
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

if(WIN32 AND ENABLE_XLS_EXPORT)
    find_package(${LEMON_QT_LIBNAME} COMPONENTS AxContainer REQUIRED)
    list(APPEND LEMON_QT_LIBS ${LEMON_QT_LIBNAME}::AxContainer)
    add_definitions(-DENABLE_XLS_EXPORT)
endif()

target_link_libraries(lemon ${LEMON_QT_LIBS} lemon-core SingleApplication::SingleApplication)


# ==================================================================================
# Platform-dependent installation process and deployment
# ==================================================================================

if(APPLE)
    include(cmake/platforms/macos.cmake)
elseif(UNIX AND NOT APPLE AND NOT WIN32)
    include(cmake/platforms/linux.cmake)
elseif(WIN32)
    include(cmake/platforms/windows.cmake)
endif()

# ==================================================================================
# Lemon Package
# ==================================================================================

include(cmake/deployment.cmake)


LEMONLOG(VERSION_STRING)
LEMONLOG(BUILD_VERSION)
LEMONLOG(CMAKE_BUILD_TYPE)
LEMONLOG(LEMON_BUILD_INFO)
LEMONLOG(LEMON_BUILD_EXTRA_INFO)
LEMONLOG(CMAKE_INSTALL_PREFIX)
LEMONLOG(CMAKE_BINARY_DIR)
message("")
message("======== LemonLime Build Statistics ========")
message("${LEMON_BUILD_STATS}")
message("============================================")
message("")
