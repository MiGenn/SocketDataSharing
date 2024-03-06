@Echo Off

set /p ndkFullPath=< "Android NDK full path.txt"
set toolchainFullPath=%ndkFullPath%"\build\cmake\android.toolchain.cmake"

set commonArguments=-D PLATFORM_TO_BUILD_FOR=Android -G "Unix Makefiles" -D CMAKE_TOOLCHAIN_FILE=%toolchainFullPath% -D ANDROID_NATIVE_API_LEVEL=23 -S ../../ -B ../../build

cmake -D ANDROID_ABI=arm64-v8a %commonArguments%/arm64-v8a
cmake -D ANDROID_ABI=x86_64 %commonArguments%/x86_64

PAUSE