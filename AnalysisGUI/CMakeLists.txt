cmake_minimum_required(VERSION 3.14)

project(AnalysisGUI LANGUAGES CXX)

add_subdirectory(qcustomplot-fetch)

file(GLOB HEADERS_H ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB UIS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QCUSTOMPLOT_VERSION "2.1.1")

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

add_executable(AnalysisGUI
  main.cpp
  ${HEADERS_H}
  ${HEADERS_HPP}
  ${SOURCES}
  ${UIS}
)

target_link_libraries(AnalysisGUI PUBLIC Qt5::Widgets Qt5::Core qcustomplot)
target_compile_definitions(AnalysisGUI PUBLIC ANALYSISGUI_LIBRARY)
target_include_directories(AnalysisGUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
