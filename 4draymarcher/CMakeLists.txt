# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)






project ("FunTimes")
include_directories ("C:/Users/veesusmikelheir/source/repos/3rdpartylibs/include")
link_directories ("C:/Users/veesusmikelheir/source/repos/3rdpartylibs/libs")

add_subdirectory ("FunTimes")


# Include sub-projects.

