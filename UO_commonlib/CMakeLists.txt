# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

project(testCommonlib)

add_subdirectory(./CommonLib libCom)
add_subdirectory(./Net libNet)
add_subdirectory(./src bin)
