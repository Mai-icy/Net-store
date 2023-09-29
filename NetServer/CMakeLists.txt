cmake_minimum_required(VERSION 3.10)
project(main VERSION 0.1.0 LANGUAGES C CXX)


# include(CTest)
# enable_testing()

add_executable(main src/main.cpp sqlite3/sqlite3.c 
                    src/dbHandle.cpp 
                    src/FileSystemManager.cpp 
                    src/ClientSession.cpp
                    src/Server.cpp
                    src/ThreadPool.cpp)

include_directories(${CMAKE_SOURCE_DIR}/include)
target_link_libraries(main dl pthread)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
