#SET(CMAKE_BUILD_TYPE "Debug")  
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -ggdb -pthread")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -pthread")  
add_definitions(-std=c++11)

include_directories(${PROJECT_SOURCE_DIR}/CommonLib)

SET(testcommonlib testcommonlib.cpp)
add_executable(testcommonlib ${testcommonlib})
target_link_libraries(testcommonlib commonlib)
