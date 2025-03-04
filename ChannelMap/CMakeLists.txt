cmake_minimum_required(VERSION 3.5)

project(ChannelMap LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets Core REQUIRED)

cmake_minimum_required(VERSION 3.5)
file(GLOB HEADERS_HPP ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
file(GLOB HEADERS_H ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB UIS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui)
# file(GLOB HEADERS_HPP ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

add_executable(ChannelMap
    ChannelMap.cpp
  ${HEADERS_H}
  ${HEADERS_HPP}
  ${SOURCES}
  ${UIS}
)
target_link_libraries(ChannelMap PUBLIC Qt5::Widgets Qt5::Core)
target_include_directories(ChannelMap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(ChannelMapGenerator
    ChannelMapGenerator.cpp
  ${HEADERS}
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ChannelMap.txt
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(ChannelMapGenerator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

