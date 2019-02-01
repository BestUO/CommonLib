#SET(CMAKE_BUILD_TYPE "Debug")  
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -ggdb")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2") 
add_compile_options(-std=c++17) 
#add_definitions(-DASIO_STANDALONE)

include_directories(${PROJECT_SOURCE_DIR}/CommonLib)
include_directories(${PROJECT_SOURCE_DIR}/Net)

SET(TESTCOMMONLIB testcommonlib.cpp)
add_executable(testcommonlib ${TESTCOMMONLIB})
target_link_libraries(testcommonlib common net pthread)
