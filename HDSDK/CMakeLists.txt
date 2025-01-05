cmake_minimum_required(VERSION 3.5)

# 设置交叉编译器
#set(CMAKE_CXX_COMPILER /tmp/g++)
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

project(HDSDK LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(Protocol)

set(ENABLE_DEMO OFF)
if(ENABLE_DEMO)
    add_executable(sendXml demo/sendXml.cpp)
    target_include_directories(sendXml PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocol
    )
    target_link_libraries(sendXml PRIVATE HDSDK)

    add_executable(sendFile demo/sendFile.cpp)
    target_include_directories(sendFile PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocol
    )
    target_link_libraries(sendFile PRIVATE HDSDK)

    add_executable(lightInfo demo/lightInfo.cpp)
    target_include_directories(lightInfo PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocol
        ${CMAKE_CURRENT_SOURCE_DIR}/SDK
    )
    target_link_libraries(lightInfo PRIVATE HDSDK)

    add_executable(systemVolume demo/systemVolume.cpp)
    target_include_directories(systemVolume PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocol
        ${CMAKE_CURRENT_SOURCE_DIR}/SDK
    )
    target_link_libraries(systemVolume PRIVATE HDSDK)

    add_executable(tcpServer demo/tcpServer.cpp)
    target_include_directories(tcpServer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocol
        ${CMAKE_CURRENT_SOURCE_DIR}/SDK
    )
    target_link_libraries(tcpServer PRIVATE HDSDK)

    add_executable(time demo/time.cpp)
    target_include_directories(time PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocol
        ${CMAKE_CURRENT_SOURCE_DIR}/SDK
    )
    target_link_libraries(time PRIVATE HDSDK)

    add_executable(eth demo/eth.cpp)
    target_include_directories(eth PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Protocol
        ${CMAKE_CURRENT_SOURCE_DIR}/SDK
    )
    target_link_libraries(eth PRIVATE HDSDK)
endif()
