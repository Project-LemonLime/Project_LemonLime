# Packaging
set(CPACK_PACKAGE_NAME "LemonLime")
set(CPACK_PACKAGE_VENDOR "Coelacanthus")
set(CPACK_PACKAGE_VERSION ${VERSION_STRING})
set(CPACK_PACKAGE_DESCRIPTION "A tiny judging environment for OI contest based on Lemon + LemonPlus.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Project-LemonLime/Project_LemonLime")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/assets/lemon-lime.ico")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSES")

if(UNIX)
    if(BUILD_DEB)
        set(CPACK_GENERATOR "DEB")
        set(CPACK_DEBIAN_PACKAGE_NAME "LemonLime")
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
