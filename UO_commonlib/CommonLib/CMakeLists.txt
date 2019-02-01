set(LIB_SRC 
	UO_Queue.h
	UO_MemoryPool.h
	UO_Public.cpp
	UO_Tree.cpp
	avlLib.cpp
	UO_Alarm.cpp
	UO_Thread.cpp
	UO_Filter.h
)
include_directories(${PROJECT_SOURCE_DIR}/Net)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -ggdb")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2")  
add_definitions(-std=c++17)
add_library(common ${LIB_SRC})
set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(common PROPERTIES OUTPUT_NAME "common")
