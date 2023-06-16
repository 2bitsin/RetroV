cmake_minimum_required(VERSION 3.22)
project(HvDOS)

file(GLOB_RECURSE SOURCE_FILES source/*.cpp source/*.hpp)

add_executable(HvDOS ${SOURCE_FILES})
target_include_directories(HvDOS PRIVATE source)
target_compile_features(HvDOS PRIVATE cxx_std_23)
find_library(LIB_WINHVPLATFORM NAMES WinHvPlatform)
target_link_libraries(HvDOS PRIVATE ${LIB_WINHVPLATFORM})

install (TARGETS HvDOS DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

message(WARNING ${SOURCE_FILES})
