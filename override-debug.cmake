# The file you specified will be read before the platform file.

# Then in override.cmake you can set your flags.  Note the _INIT
# suffix on the variable names.
SET(CMAKE_C_FLAGS_INIT " -march=armv7-a -mfpu=neon  -mthumb -mthumb-interwork -mfloat-abi=softfp -DKLAATU -D_USE_MATH_DEFINES  -DUSE_ARM_NEON -DUSE_ARM_SIMD -include limits.h  --sysroot=$ENV{ANDROID_BUILD_TOP}")
SET(CMAKE_C_FLAGS_DEBUG_INIT "-march=armv7-a -mfpu=neon  -mthumb -mthumb-interwork -mfloat-abi=softfp -DKLAATU -D_USE_MATH_DEFINES  -DUSE_ARM_NEON -DUSE_ARM_SIMD -include limits.h  --sysroot=$ENV{ANDROID_BUILD_TOP}")
SET(CMAKE_C_FLAGS_RELEASE_INIT "-march=armv7-a -mfpu=neon  -mthumb -mthumb-interwork -mfloat-abi=softfp -DKLAATU -D_USE_MATH_DEFINES  -DUSE_ARM_NEON -DUSE_ARM_SIMD -include limits.h  --sysroot=$ENV{ANDROID_BUILD_TOP}")
SET(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-march=armv7-a -mfpu=neon  -mthumb -mthumb-interwork -mfloat-abi=softfp -DKLAATU -D_USE_MATH_DEFINES  -DUSE_ARM_NEON -DUSE_ARM_SIMD -include limits.h  --sysroot=$ENV{ANDROID_BUILD_TOP}")
SET(CMAKE_CXX_FLAGS_INIT "-frtti -march=armv7-a -mfpu=neon  -mthumb -mthumb-interwork -mfloat-abi=softfp -DKLAATU -D_USE_MATH_DEFINES  -DUSE_ARM_NEON -DUSE_ARM_SIMD -include limits.h  -I$ENV{ANDROID_NDK_TOP}/sources/cxx-stl/gnu-libstdc++/4.6/include -I$ENV{ANDROID_NDK_TOP}/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a/include  -I$ENV{ANDROID_BUILD_TOP}/usr/include/bionic -DANDROID --sysroot=$ENV{ANDROID_BUILD_TOP} ")
SET(CMAKE_CXX_FLAGS_DEBUG_INIT "-frtti -march=armv7-a -mfpu=neon  -mthumb -mthumb-interwork -mfloat-abi=softfp -DKLAATU -D_USE_MATH_DEFINES  -DUSE_ARM_NEON -DUSE_ARM_SIMD -include limits.h  -I$ENV{ANDROID_NDK_TOP}/sources/cxx-stl/gnu-libstdc++/4.6/include -I$ENV{ANDROID_NDK_TOP}/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a/include  -I$ENV{ANDROID_BUILD_TOP}/usr/include/bionic -DANDROID --sysroot=$ENV{ANDROID_BUILD_TOP} ")
SET(CMAKE_CXX_FLAGS_RELEASE_INIT "-frtti -march=armv7-a -mfpu=neon  -mthumb -mthumb-interwork -mfloat-abi=softfp -DKLAATU -D_USE_MATH_DEFINES  -DUSE_ARM_NEON -DUSE_ARM_SIMD -include limits.h  -I$ENV{ANDROID_NDK_TOP}/sources/cxx-stl/gnu-libstdc++/4.6/include -I$ENV{ANDROID_NDK_TOP}/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a/include  -I$ENV{ANDROID_BUILD_TOP}/usr/include/bionic -DANDROID --sysroot=$ENV{ANDROID_BUILD_TOP} ")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-frtti -march=armv7-a -mfpu=neon  -mthumb -mthumb-interwork -mfloat-abi=softfp -DKLAATU -D_USE_MATH_DEFINES  -DUSE_ARM_NEON -DUSE_ARM_SIMD -include limits.h  -I$ENV{ANDROID_NDK_TOP}/sources/cxx-stl/gnu-libstdc++/4.6/include -I$ENV{ANDROID_NDK_TOP}/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a/include  -I$ENV{ANDROID_BUILD_TOP}/usr/include/bionic -DANDROID --sysroot=$ENV{ANDROID_BUILD_TOP} ")




SET(CMAKE_EXE_LINKER_FLAGS_INIT " -Wl,--no-keep-memory --sysroot=$ENV{ANDROID_BUILD_TOP}  -L$ENV{ANDROID_BUILD_TOP}/usr/local/lib -lgnustl_shared -llog -lm -lz ")
SET(CMAKE_SHARED_LINKER_FLAGS_INIT " -Wl,--reduce-memory-overheads -Wl,--no-keep-memory --sysroot=$ENV{ANDROID_BUILD_TOP}  -L$ENV{ANDROID_BUILD_TOP}/usr/local/lib -lgnustl_shared -llog -lm -lstdc++ -lui -lwpa_client -lz")