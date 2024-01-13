cmake_minimum_required(VERSION 3.14)

project(SqliteWrap LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create an object library for sqlite3.c
add_library(Sqlite3Object OBJECT sqlite3.c)
set_property(TARGET Sqlite3Object PROPERTY POSITION_INDEPENDENT_CODE ON)  # Set PIC flag

add_library(SqliteWrap SHARED
  SqliteWrap_global.h
  sqlitewrap.cpp
  sqlitewrap.h
  sqlite3.h
  $<TARGET_OBJECTS:Sqlite3Object>  # Link sqlite3.c object library here
)

target_compile_definitions(SqliteWrap PRIVATE SQLITEWRAP_LIBRARY)

# Link the necessary libraries
target_link_libraries(SqliteWrap PRIVATE pthread dl)
