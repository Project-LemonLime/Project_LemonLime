# Packaging
set(CPACK_PACKAGE_NAME "LemonLime")
set(CPACK_PACKAGE_VENDOR "Project LemonLime Developers")
set(CPACK_PACKAGE_DESCRIPTION "A tiny judging environment for OI contest based on Lemon + LemonPlus.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Project-LemonLime/Project_LemonLime")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/assets/lemon-lime.ico")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSES")

SET(COMMON_IGNORE_FILES "/CMakeFiles/" "_CPack_Packages/" "/Testing/"
    ".cmake$"   ".directory$" "CMakeCache.txt"
    "/.svn/"  "/CVS/" "~$" ".swp$" ".log$" ".gz$"
    "/src/config.h$")

SET(CPACK_PACKAGE_IGNORE_FILES ${PRJ_COMMON_IGNORE_FILES} )

if(UNIX)
    if(BUILD_DEB)
        set(CPACK_GENERATOR "DEB")
        set(CPACK_DEBIAN_PACKAGE_NAME "LemonLime")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Project LemonLime Developers")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "qtbase5-dev, qttools5-dev, libspdlog")
    endif()

    if(BUILD_RPM)
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/makespec/lemonlime.spec.in" "${CMAKE_CURRENT_BINARY_DIR}/lemonlime.spec" @ONLY IMMEDIATE)
        #set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_BINARY_DIR}/lemonlime.spec")
        set(CPACK_GENERATOR "RPM")
        set(CPACK_RPM_PACKAGE_NAME "LemonLime")
        set(CPACK_RPM_PACKAGE_VENDOR "Project LemonLime Developers")
        if(LSB_RELEASE_ID_SHORT EQUAL "openSUSE")
            set(CPACK_RPM_BUILDREQUIRES "cmake >= 3.9, libqt5-qtbase-common-devel >= 5.15, libQt5Core-devel, libQt5Gui-devel, libQt5Network-devel, libQt5Widgets-devel, libQt5Concurrent-devel, libqt5-linguist-devel, libqt5-qtsvg-devel, update-desktop-files, ninja, spdlog-devel")
        else()
            set(CPACK_RPM_BUILDREQUIRES "cmake >= 3.9, qt5-qtbase-devel >= 5.15, qt5-linguist, qt5-qtsvg-devel, desktop-file-utils, ninja-build, spdlog")
        endif()
        set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
    endif()
endif()

# Must Here
include(CPack)
