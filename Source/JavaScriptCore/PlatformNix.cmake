list(APPEND JavaScriptCore_SOURCES
    jit/ExecutableAllocatorFixedVMPool.cpp
    jit/ExecutableAllocator.cpp
)

list(APPEND JavaScriptCore_INCLUDE_DIRECTORIES
    ${WTF_DIR}/wtf/gobject
)
