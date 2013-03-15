include(CMakeForceCompiler)
set(CMAKE_CROSSCOMPILING   TRUE)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR "armv7-a")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -march=armv7-a -mfpu=neon  -mthumb -mthumb-interwork -mfloat-abi=softfp -DKLAATU -D_USE_MATH_DEFINES  -DUSE_ARM_NEON -DUSE_ARM_SIMD -include limits.h  -I$ENV{ANDROID_NDK_TOP}/sources/cxx-stl/gnu-libstdc++/4.6/include -I$ENV{ANDROID_NDK_TOP}/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi-v7a/include  -I$ENV{ANDROID_BUILD_TOP}/usr/include/bionic -DANDROID --sysroot=$ENV{ANDROID_BUILD_TOP} ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -march=armv7-a -mfpu=neon  -mthumb -mthumb-interwork -mfloat-abi=softfp -DKLAATU -D_USE_MATH_DEFINES  -DUSE_ARM_NEON -DUSE_ARM_SIMD -include limits.h  --sysroot=$ENV{ANDROID_BUILD_TOP}" )
set(CMAKE_C_COMPILER arm-linux-gcc)
set(CMAKE_CXX_COMPILER arm-linux-g++)
CMAKE_FORCE_C_COMPILER("${CMAKE_C_COMPILER}" GNU)
CMAKE_FORCE_CXX_COMPILER("${CMAKE_CXX_COMPILER}" GNU)
set(CMAKE_FIND_ROOT_PATH  $ENV{ANDROID_BUILD_TOP})
