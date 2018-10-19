cmake_minimum_required(VERSION 2.6)
project(mathdb)
file(GLOB src "*.h" "*.c")
SET(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Werror -Wextra -pedantic -
Wshadow -Woverloaded-virtual -Winvalid-pch -Wcast-align -Wformat=2 -
Wformat-nonliteral -Wmissing-declarations -Wmissing-format-attribute -
Wmissing-include-dirs -Wredundant-decls -Wswitch-default")
add_executable(test ${src})
