cmake_minimum_required(VERSION 3.18)
project(sqlList VERSION 1.0)

set(CMAKE_C_STANDARD 11)

add_executable(sqlList
        main.c
        sqlite3.h)

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})

TARGET_LINK_LIBRARIES(sqlList ${GTK3_LIBRARIES} sqlite3)
add_compile_options(-l sqlite3)
