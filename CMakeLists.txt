cmake_minimum_required (VERSION 3.1)
project (lunatic)

add_subdirectory(vendor/libtwittercpp)

find_package(PkgConfig)
pkg_check_modules(yaml-cpp yaml-cpp REQUIRED)
pkg_check_modules(GraphicsMagick GraphicsMagick++ REQUIRED)
pkg_check_modules(sqlite3 sqlite3>=3.8.3 REQUIRED)

include_directories(
  vendor/libtwittercpp/src
  ${yaml-cpp_INCLUDE_DIRS}
  ${GraphicsMagick_INCLUDE_DIRS})

link_directories(${GraphicsMagick_LIBRARY_DIRS})

add_executable(lunatic lunatic.cpp database.cpp)
set_property(TARGET lunatic PROPERTY CXX_STANDARD 11)
set_property(TARGET lunatic PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(lunatic ${yaml-cpp_LIBRARIES} ${GraphicsMagick_LIBRARIES} ${sqlite3_LIBRARIES} twitter++)