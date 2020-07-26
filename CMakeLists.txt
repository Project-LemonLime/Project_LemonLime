cmake_minimum_required(VERSION 3.9.0 )


file(STRINGS "${CMAKE_SOURCE_DIR}/makespec/VERSION" VERSION)
file(STRINGS "${CMAKE_SOURCE_DIR}/makespec/BUILDVERSION" BUILD_VERSION)
file(STRINGS "${CMAKE_SOURCE_DIR}/makespec/VERSIONSUFFIX" VERSION_SUFFIX)

project(lemon)

set(VERSION_STRING "${VERSION}${VERSION_SUFFIX}")
set(VERSION_LIST ${VERSION})
string(REPLACE "." ";" VERSION_LIST ${VERSION_LIST})
separate_arguments(VERSION_LIST)

list(GET VERSION_LIST 0 CMAKE_PROJECT_VERSION_MAJOR)
list(GET VERSION_LIST 1 CMAKE_PROJECT_VERSION_MINOR)
list(GET VERSION_LIST 2 CPACK_PACKAGE_VERSION_PATCH)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/src/forms)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#add_link_options(-no-pie)

set(SINGLEAPPLICATION_DIR ${CMAKE_SOURCE_DIR}/3rdparty/SingleApplication)
set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")
add_subdirectory(3rdparty/SingleApplication)

if(WIN32)
    find_package(Qt5 COMPONENTS AxContainer REQUIRED)
    add_compile_options("/std:c++11")
    add_compile_definitions(UNICODE _UNICODE)
endif()

message(" ")
message("LemonLime Version: ${VERSION_STRING}")
message("LemonLime Build Version: ${BUILD_VERSION}")
message("LemonLime Build Type: ${CMAKE_BUILD_TYPE}")
message("|-----------------------------------------------------|")
message("| LemonLime,                                          |")
message("| A tiny judging environment for OI contest.          |")
message("| Based on C++ & Qt.                                  |")
message("| Licenced under GPLv3                                 |")
message("|                                                     |")
message("|-----------------------------------------------------|")
message("| GitHub: https://github.com/iotang/Project_LemonLime |")
message("| Welcome to contribute!                              |")
message("|-----------------------------------------------------|")
message(" ")

if(WIN32)
    find_package(Qt5 COMPONENTS AxContainer REQUIRED)
    add_compile_definitions(UNICODE _UNICODE)
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)
if(UNIX)
    if(Qt5Core_VERSION VERSION_LESS  5.4.0)
        execute_process(COMMAND sed -i '/<property name="tabBarAutoHide">/,+2d' ${CMAKE_SOURCE_DIR}/src/forms/lemon.ui)
    endif()
endif()

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

set(RESOURCES
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

include(cmake/translations.cmake)

# Qt language translations
add_custom_target(lupdate
    COMMAND lupdate ${HEADERS}
    ${SRC}
    ${RESOURCES}
    ${SINGLEAPPLICATION_SOURCES} -ts ${TRANSLATIONS_TS} -locations none -noobsolete
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_target_properties(lupdate PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_executable(lemon ${HEADERS} ${SRC} resource.qrc)
target_include_directories(lemon PUBLIC ${CMAKE_SOURCE_DIR}/src)
if(WIN32)
    target_link_libraries(lemon Qt5::Widgets Qt5::Gui Qt5::Network Qt5::Core Qt5::AxContainer SingleApplication::SingleApplication)
else()
    target_link_libraries(lemon Qt5::Widgets Qt5::Gui Qt5::Network Qt5::Core SingleApplication::SingleApplication)
endif()


if(UNIX AND NOT APPLE AND NOT WIN32)
    install(TARGETS lemon RUNTIME DESTINATION bin)
    install(FILES assets/lemon-lime.desktop DESTINATION share/applications)
    install(FILES assets/lemon-lime.png DESTINATION share/icons/hicolor/256x256/apps)
endif()



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
