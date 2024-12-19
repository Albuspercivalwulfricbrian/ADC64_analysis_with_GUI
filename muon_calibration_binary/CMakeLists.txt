# CMakeLists.txt for event package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(entanglement LANGUAGES CXX)

add_subdirectory(ChannelMap)
file(GLOB HEADERS_HPP ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
file(GLOB HEADERS_H ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB MAP_H ${CMAKE_CURRENT_SOURCE_DIR}/ChannelMap/ChannelMap/include/*.h*)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS RIO Core Tree Net Hist Graf Gpad Gui Physics)
find_package (Eigen3 3.3 REQUIRED NO_MODULE)

include(${ROOT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
${CMAKE_CURRENT_SOURCE_DIR}/src)
ROOT_GENERATE_DICTIONARY(G__CustomClassDictionary ${HEADERS_H} LINKDEF LinkDef.h)

add_library(CustomClassDictionary SHARED G__CustomClassDictionary.cxx ${HEADERS_H} ${HEADERS_HPP} ${SOURCES})

add_executable(FileReader FileReader.cpp ${SOURCES})
target_link_libraries(FileReader CustomClassDictionary ROOT::RIO ROOT::Core ROOT::Tree ROOT::Net ROOT::Hist ROOT::Graf ROOT::Gpad ROOT::Physics)

add_executable(CreateTrackTree CreateTrackTree.C ${HEADERS_H} ${HEADERS_HPP} ${SOURCES} ${MAP_H})
target_link_libraries(CreateTrackTree CustomClassDictionary ROOT::RIO ROOT::Core ROOT::Tree ROOT::Net ROOT::Hist ROOT::Graf ROOT::Gpad ROOT::Gui ROOT::Physics)

add_executable(Calibrator Calibrator.C ${HEADERS_H} ${HEADERS_HPP} ${SOURCES})
target_link_libraries(Calibrator PUBLIC CustomClassDictionary ROOT::RIO ROOT::Core ROOT::Tree ROOT::Net ROOT::Hist ROOT::Graf ROOT::Gpad ROOT::Physics)
