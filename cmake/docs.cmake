if(EMBED_DOCS)
    add_definitions(-DLEMON_EMBED_DOCS)
    set(LEMON_EMBED_DOC_QRC ${CMAKE_SOURCE_DIR}/manual/manual.qrc)
endif()
