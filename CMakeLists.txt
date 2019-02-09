#This is an example CMakeFile
cmake_minimum_required(VERSION 2.8)
add_compile_options(-std=c++11)

project(piano_tiles)

include_directories( $ENV{HOME}/midifile/include/)

message($ENV{HOME}/midifile/include/)

#specify your source files here, add more if needed
add_executable(${PROJECT_NAME} "main.cpp")


target_link_libraries(${PROJECT_NAME} $ENV{HOME}/midifile/lib/libmidifile.a )
