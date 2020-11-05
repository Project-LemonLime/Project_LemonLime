# ==================================================================================
# Lemon Translations
# ==================================================================================

find_package(${LEMON_QT_LIBNAME} COMPONENTS LinguistTools REQUIRED)
set(TRANSLATIONS_DIR ${CMAKE_SOURCE_DIR}/translations)
file(GLOB TRANSLATIONS_TS ${TRANSLATIONS_DIR}/**.ts)
if(LEMON_QT6)
    message(WARNING "Translations are not supported currently.")
else()
    qt5_add_translation(QM_FILES ${TRANSLATIONS_TS})
endif()
