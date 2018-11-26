# CMake file for compiling the sqlite3 static library under Windows (for ease of use).
add_library(sqlite3
 sqlite3.c
 sqlite3.h
 sqlite3ext.h
)
