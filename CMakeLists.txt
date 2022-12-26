cmake_minimum_required(VERSION 3.23)
project(C1)

set(CMAKE_CXX_STANDARD 14)

add_executable(D1V1 D1/src/var1.cpp)
add_executable(D1V2 D1/src/var2.cpp)
add_executable(D1V3 D1/src/var3.cpp)
add_executable(D1V4 D1/src/var4.cpp)
add_executable(D1V5 D1/src/var5.cpp)

add_executable(D2V1 D2/src/ch2.cpp)

add_executable(D3V1 "D3/CH3 v1/CH3Var1_T142.cpp")
add_executable(D3V3 "D3/CH3 v3/CH3Var3_T142.cpp")
add_executable(D3V4 "D3/CH3 v4/CH3Var4_T142.cpp")

add_executable(D4V1 D4/src/Var3.cpp)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# project
add_executable(Project Project/src/CProject_T142.cpp)
target_link_libraries(Project ${CURSES_LIBRARIES})
