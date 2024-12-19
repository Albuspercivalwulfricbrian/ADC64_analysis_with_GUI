cmake_minimum_required(VERSION 3.5)

project(ChannelMap LANGUAGES CXX)

file(GLOB HEADERS_HPP ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
file(GLOB HEADERS_H ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB UIS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

add_executable(ChannelMap
  main.cpp
  ${HEADERS_H}
  ${HEADERS_HPP}
  ${SOURCES}
  ${UIS}
)
target_link_libraries(ChannelMap PUBLIC Qt5::Widgets Qt5::Core)
target_include_directories(ChannelMap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
