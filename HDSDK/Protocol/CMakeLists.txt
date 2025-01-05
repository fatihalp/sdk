cmake_minimum_required(VERSION 3.5)

project(HDSDK LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(ENABLE_SDK ON)
if (ENABLE_SDK)
    list(APPEND SDK_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/tinyxml2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/HXml.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/SDKInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Data/HSDKInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Data/HEthernetInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Data/HGeneralInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Data/HLightInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Data/HOTherInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Data/HScreenFunctionInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Data/HSDKInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Data/HSensorInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Data/HTimeInfo.cpp
    )
endif()

add_library(HDSDK SHARED
    HDSDK.cpp
    ${SDK_FILES}
)

if (ENABLE_SDK)
    target_include_directories(HDSDK PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../SDK)
    target_include_directories(HDSDK PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Data)
endif()

target_include_directories(HDSDK PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(HDSDK PRIVATE -static-libgcc -static-libstdc++)
set_target_properties(HDSDK PROPERTIES LINK_FLAGS "-nodefaultlibs")
set(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB")
add_definitions(-DUSE_HD_LIB)
