set(LIB_SRC 
	UO_Queue.h
	UO_MemoryPool.h
	UO_Public.h
	UO_Tree.cpp
	avlLib.cpp
)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -ggdb -pthread")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -pthread")  
add_definitions(-std=c++11)
add_library(commonlib ${LIB_SRC})
set_target_properties(commonlib PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(commonlib PROPERTIES OUTPUT_NAME "common")
