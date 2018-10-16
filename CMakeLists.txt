cmake_minimum_required(VERSION 2.6)
project(mathdb)
file(GLOB src "*.h" "*.c")
add_executable(test ${src})
