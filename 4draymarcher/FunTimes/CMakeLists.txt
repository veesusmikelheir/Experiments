# CMakeList.txt : CMake project for FunTimes, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (FunTimes "FunTimes.cpp" "FunTimes.h" "glad.c" "utils.cpp" "utils.h" "ECSSystem.cpp" "ECSSystem.h")

target_link_libraries(FunTimes glfw3)
target_link_libraries(FunTimes opengl32.lib)
# TODO: Add tests and install targets if needed.
