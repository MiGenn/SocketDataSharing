cmake_minimum_required(VERSION 3.28.3)

project(SocketDataSharing)
set(PLATFORM_TO_BUILD_FOR null CACHE STRING "Platform to build for")

set(NEEDED_INCLUDE_DIRECTORIES source/common/include)
set(NEEDED_LIBRARIES "")
set(NEEDED_SOURCE_FILES 
    source/common/include/interface/SocketDataSharing.hpp 
    source/common/include/interface/Error.hpp 
    source/common/source/Error.cpp 
    source/common/include/ErrorHandler.hpp 
    source/common/source/ErrorHandler.cpp 
    )

if(${PLATFORM_TO_BUILD_FOR} STREQUAL Windows)
    if (NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
        message(FATAL_ERROR "In order to build for Windows you need to be on Windows!")
    endif()

    list(APPEND NEEDED_INCLUDE_DIRECTORIES source/windows/include)
    list(APPEND NEEDED_LIBRARIES Ws2_32.lib Iphlpapi.lib)
    list(APPEND NEEDED_SOURCE_FILES 
        source/windows/source/SocketDataSharing.cpp 
        source/windows/include/WinAPI.hpp 
        source/windows/source/ErrorHandlerWindowsDefinitions.cpp 
        )
elseif(${PLATFORM_TO_BUILD_FOR} STREQUAL Android)
    if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL Android)
        message(FATAL_ERROR "In order to build for Android you need to specify a cmake toolchain file path from a downloaded Android NDK!")
    endif()

    list(APPEND NEEDED_INCLUDE_DIRECTORIES source/android/include)
    list(APPEND NEEDED_LIBRARIES android log)
    list(APPEND NEEDED_SOURCE_FILES 
        source/android/source/SocketDataSharing.cpp 
        source/android/source/ErrorHandlerAndroidDefinitions.cpp 
        )
else()
    message(FATAL_ERROR "PLATFORM_TO_BUILD_FOR value is invalid. The value is ${PLATFORM_TO_BUILD_FOR}!")
endif()

add_library(${PROJECT_NAME}Static STATIC ${NEEDED_SOURCE_FILES})
set_target_properties(${PROJECT_NAME}Static PROPERTIES OUTPUT_NAME ${PROJECT_NAME}_static LINKER_LANGUAGE CXX)
target_compile_features(${PROJECT_NAME}Static PRIVATE cxx_std_17)
target_compile_definitions(${PROJECT_NAME}Static PRIVATE SOCKETDATASHARING_EXPORTS)
target_include_directories(${PROJECT_NAME}Static PRIVATE ${NEEDED_INCLUDE_DIRECTORIES})
target_link_libraries(${PROJECT_NAME}Static PRIVATE ${NEEDED_LIBRARIES})

add_library(${PROJECT_NAME}Dynamic SHARED ${NEEDED_SOURCE_FILES})
set_target_properties(${PROJECT_NAME}Dynamic PROPERTIES OUTPUT_NAME ${PROJECT_NAME} LINKER_LANGUAGE CXX)
target_compile_features(${PROJECT_NAME}Dynamic PRIVATE cxx_std_17)
target_compile_definitions(${PROJECT_NAME}Dynamic PRIVATE SOCKETDATASHARING_EXPORTS)
target_include_directories(${PROJECT_NAME}Dynamic PRIVATE ${NEEDED_INCLUDE_DIRECTORIES})
target_link_libraries(${PROJECT_NAME}Dynamic PRIVATE ${NEEDED_LIBRARIES})