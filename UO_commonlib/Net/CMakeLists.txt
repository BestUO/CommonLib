cmake_minimum_required(VERSION 2.8)
set(LIB_SRC
	UO_Net.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/Net)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -ggdb")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2")  
add_compile_options(-std=c++17) 
#add_definitions(-DASIO_STANDALONE)
add_library(net ${LIB_SRC})
set_target_properties(net PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(net PROPERTIES OUTPUT_NAME "net")
