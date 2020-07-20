cmake_minimum_required(VERSION 3.9.0 )

project(lemon)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/forms)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(WIN32)
    add_compile_options("/std:c++11")
endif()
add_link_options(-no-pie)

set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")
add_subdirectory(singleapplication)

if(WIN32)
    find_package(Qt5 COMPONENTS Axcontainer REQUIRED)
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)

execute_process(COMMAND pkg-config --modversion Qt5Core
				OUTPUT_VARIABLE QT_5_VERSION
				)

if(QT_5_VERSION VERSION_LESS 5.4)
	execute_process(COMMAND ${CMAKE_SOURCE_DIR}/tools/qt_5_4.sh ${CMAKE_SOURCE_DIR})
elseif(QT_5_VERSION VERSION_LESS 5.5)
	execute_process(COMMAND ${CMAKE_SOURCE_DIR}/tools/qt_5_5.sh ${CMAKE_SOURCE_DIR})
elseif(QT_5_VERSION VERSION_LESS 5.7)
	execute_process(COMMAND ${CMAKE_SOURCE_DIR}/tools/qt_5_7.sh ${CMAKE_SOURCE_DIR})
elseif(QT_5_VERSION VERSION_LESS 5.13)
	execute_process(COMMAND ${CMAKE_SOURCE_DIR}/tools/qt_5_13.sh ${CMAKE_SOURCE_DIR})
elseif(QT_5_VERSION VERSION_LESS 5.14)
	execute_process(COMMAND ${CMAKE_SOURCE_DIR}/tools/qt_5_14.sh ${CMAKE_SOURCE_DIR})
endif()

set(SRC 
	main.cpp
	exttestcasemodifier.cpp
	exttestcasemodifierdialog.cpp
	exttestcasetable.cpp
	exttestcaseupdaterdialog.cpp
	visualsettings.cpp
	lemon.cpp
	contest.cpp
	task.cpp
	testcase.cpp
	settings.cpp
	compiler.cpp
	filelineedit.cpp
	summarytree.cpp
	taskeditwidget.cpp
	testcaseeditwidget.cpp
	generalsettings.cpp
	compilersettings.cpp
	addtestcaseswizard.cpp
	contestant.cpp
	judgingdialog.cpp
	judgingthread.cpp
	optionsdialog.cpp
	resultviewer.cpp
	assignmentthread.cpp
	detaildialog.cpp
	newcontestwidget.cpp
	opencontestwidget.cpp
	newcontestdialog.cpp
	opencontestdialog.cpp
	welcomedialog.cpp
	addtaskdialog.cpp
	advancedcompilersettingsdialog.cpp
	environmentvariablesdialog.cpp
	editvariabledialog.cpp
	addcompilerwizard.cpp
	exportutil.cpp
	statisticsbrowser.cpp
	)
	
set(HEADERS 
	lemon.h
	exttestcasemodifier.h
	contest.h
	exttestcasemodifierdialog.h
	exttestcasetable.h
	exttestcaseupdaterdialog.h
	subtaskdependencelib.h
	visualsettings.h
	task.h
	testcase.h
	settings.h
	compiler.h
	filelineedit.h
	summarytree.h
	taskeditwidget.h
	testcaseeditwidget.h
	generalsettings.h
	compilersettings.h
	addtestcaseswizard.h
	contestant.h
	judgingdialog.h
	judgingthread.h
	optionsdialog.h
	resultviewer.h
	assignmentthread.h
	globaltype.h
	detaildialog.h
	newcontestwidget.h
	opencontestwidget.h
	newcontestdialog.h
	opencontestdialog.h
	welcomedialog.h
	addtaskdialog.h
	advancedcompilersettingsdialog.h
	environmentvariablesdialog.h
	editvariabledialog.h
	addcompilerwizard.h
	exportutil.h
	statisticsbrowser.h
	judgesharedvariables.h
	)

set(RESOURCES 
	forms/lemon.ui
	forms/exttestcasemodifierdialog.ui
	forms/exttestcasemodifier.ui
	forms/exttestcaseupdaterdialog.ui
	forms/taskeditwidget.ui
	forms/testcaseeditwidget.ui
	forms/generalsettings.ui
	forms/compilersettings.ui
	forms/addtestcaseswizard.ui
	forms/judgingdialog.ui
	forms/optionsdialog.ui
	forms/detaildialog.ui
	forms/newcontestwidget.ui
	forms/opencontestwidget.ui
	forms/newcontestdialog.ui
	forms/opencontestdialog.ui
	forms/visualsettings.ui
	forms/welcomedialog.ui
	forms/addtaskdialog.ui
	forms/advancedcompilersettingsdialog.ui
	forms/environmentvariablesdialog.ui
	forms/editvariabledialog.ui
	forms/addcompilerwizard.ui
	forms/statisticsbrowser.ui
	)

	
add_executable(lemon ${HEADERS} ${SRC} resource.qrc)
if(WIN32)
    target_link_libraries(lemon Qt5::Widgets Qt5::Gui Qt5::Network Qt5::Core Qt5::Axcontainer SingleApplication::SingleApplication)
else()
    target_link_libraries(lemon Qt5::Widgets Qt5::Gui Qt5::Network Qt5::Core SingleApplication::SingleApplication)
endif()
