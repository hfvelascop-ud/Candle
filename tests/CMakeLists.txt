cmake_minimum_required(VERSION 3.10)
project(CandleHeightmapTests LANGUAGES CXX)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt5 REQUIRED COMPONENTS Core)
enable_testing()
add_executable(heightmap-interpolation-test
    heightmap_interpolation_test.cpp
    ../src/candle/tables/heightmaptablemodel.cpp
    ../src/candle/tables/heightmaptablemodel.h)
target_include_directories(heightmap-interpolation-test PRIVATE ../src/candle)
target_link_libraries(heightmap-interpolation-test PRIVATE Qt5::Core)
add_test(NAME heightmap-interpolation COMMAND heightmap-interpolation-test)
