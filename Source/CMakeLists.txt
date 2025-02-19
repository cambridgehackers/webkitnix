# -----------------------------------------------------------------------------
# Add module directories
# -----------------------------------------------------------------------------
add_subdirectory(WTF)

add_subdirectory(JavaScriptCore)

if ("${PORT}" STREQUAL "Nix")
    add_subdirectory(Platform)
endif ()

if (ENABLE_API_TESTS)
    add_subdirectory(cmake/gtest)
endif ()

if (ENABLE_WEBCORE)
    add_subdirectory(WebCore)
endif ()

if (ENABLE_WEBKIT)
    add_subdirectory(WebKit)
endif ()

if (ENABLE_WEBKIT2)
    add_subdirectory(WebKit2)
endif ()

WEBKIT_INCLUDE_CONFIG_FILES_IF_EXISTS()

# -----------------------------------------------------------------------------
# Set compiler flags for all targets
# -----------------------------------------------------------------------------
if ("${PORT}" STREQUAL "Nix")
    WEBKIT_SET_EXTRA_COMPILER_FLAGS(${Platform_LIBRARY_NAME})
endif ()

if (${PORT} STREQUAL "Efl")
    set(ADDITIONAL_FLAGS ENABLE_WERROR)
endif ()

WEBKIT_SET_EXTRA_COMPILER_FLAGS(${WTF_LIBRARY_NAME} ${ADDITIONAL_FLAGS})
WEBKIT_SET_EXTRA_COMPILER_FLAGS(${JavaScriptCore_LIBRARY_NAME} ${ADDITIONAL_FLAGS})
WEBKIT_SET_EXTRA_COMPILER_FLAGS(${WebCoreTestSupport_LIBRARY_NAME} ${ADDITIONAL_FLAGS})

if (ENABLE_WEBCORE)
    WEBKIT_SET_EXTRA_COMPILER_FLAGS(${WebCore_LIBRARY_NAME} ${ADDITIONAL_FLAGS})
endif ()

if (ENABLE_WEBKIT)
    WEBKIT_SET_EXTRA_COMPILER_FLAGS(${WebKit_LIBRARY_NAME} ${ADDITIONAL_FLAGS})
endif ()

if (ENABLE_WEBKIT2)
    WEBKIT_SET_EXTRA_COMPILER_FLAGS(${WebKit2_LIBRARY_NAME} ${ADDITIONAL_FLAGS})
endif ()

# -----------------------------------------------------------------------------
# Create the configuration file
# -----------------------------------------------------------------------------
if (NOT "${PORT}" STREQUAL "Nix")
    configure_file(cmakeconfig.h.cmake ${CMAKE_BINARY_DIR}/cmakeconfig.h @ONLY)
endif ()
