@Echo Off

cmake -D PLATFORM_TO_BUILD_FOR=Windows -G "Visual Studio 17 2022" -A x64 -S ../../ -B ../../build

PAUSE