list(APPEND WTF_SOURCES
    gtk/MainThreadGtk.cpp
    gobject/GOwnPtr.cpp
    gobject/GRefPtr.cpp
    gobject/GlibUtilities.cpp

    OSAllocatorPosix.cpp
    ThreadIdentifierDataPthreads.cpp
    ThreadingPthreads.cpp

)

list(APPEND WTF_LIBRARIES
    pthread
    ${GLIB_LIBRARIES}
    ${GLIB_GIO_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

list(APPEND WTF_INCLUDE_DIRECTORIES
    ${GLIB_INCLUDE_DIRS}
    ${JAVASCRIPTCORE_DIR}/wtf/gobject
    ${JAVASCRIPTCORE_DIR}/wtf/unicode/
)
