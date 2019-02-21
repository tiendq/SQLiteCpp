set(SQLITECPP_INTERNAL_SQLITE OFF CACHE BOOL "Build internal SQLite3")
set(SQLITECPP_RUN_DOXYGEN OFF CACHE BOOL "Build documentation with Doxygen")
set(SQLITECPP_BUILD_EXAMPLES OFF CACHE BOOL "Build examples")
set(SQLITECPP_BUILD_TESTS OFF CACHE BOOL "Build unit tests")

set(TARGET_NAME SQLiteCpp)

set(SQLITECPP_SOURCES
  Backup.cpp
  Column.cpp
  Database.cpp
  Exception.cpp
  Statement.cpp
  Transaction.cpp
)

set(SQLITECPP_HEADERS
  ../include/SQLiteCpp/SQLiteCpp.h
  ../include/SQLiteCpp/Assertion.h
  ../include/SQLiteCpp/Backup.h
  ../include/SQLiteCpp/Column.h
  ../include/SQLiteCpp/Database.h
  ../include/SQLiteCpp/Exception.h
  ../include/SQLiteCpp/Statement.h
  ../include/SQLiteCpp/Transaction.h
  ../include/SQLiteCpp/Utils.h
  ../include/SQLiteCpp/VariadicBind.h
)

add_library(${TARGET_NAME} ${SQLITECPP_SOURCES} ${SQLITECPP_HEADERS})
add_library("${TARGET_NAME}::${TARGET_NAME}" ALIAS ${TARGET_NAME})

# Options relative to SQLite and SQLiteC++ functions
option(SQLITE_ENABLE_COLUMN_METADATA "Enable Column::getColumnOriginName(). Require support from sqlite3 library." ON)

if(SQLITE_ENABLE_COLUMN_METADATA)
  # Enable the use of SQLite column metadata and Column::getColumnOriginName() method,
  # Require that the sqlite3 library is also compiled with this flag (default under Debian/Ubuntu, but not on Mac OS X).
  target_compile_definitions(${TARGET_NAME} PRIVATE SQLITE_ENABLE_COLUMN_METADATA)
endif()

option(SQLITE_ENABLE_ASSERT_HANDLER "Enable the user defintion of a assertion_failed() handler." OFF)
if(SQLITE_ENABLE_ASSERT_HANDLER)
  # Enable the user defintion of a assertion_failed() handler (default to false, easier to handler for begginers).
  target_compile_definitions(${TARGET_NAME} PRIVATE SQLITECPP_ENABLE_ASSERT_HANDLER)
endif()

option(SQLITE_USE_LEGACY_STRUCT "Fallback to forward declaration of legacy struct sqlite3_value (pre SQLite 3.19)" OFF)
if(SQLITE_USE_LEGACY_STRUCT)
  # Force forward declaration of legacy struct sqlite3_value (pre SQLite 3.19)
  target_compile_definitions(${TARGET_NAME} PRIVATE SQLITE_USE_LEGACY_STRUCT)
endif()

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

target_include_directories(${TARGET_NAME}
  PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
  SYSTEM INTERFACE
    "${CMAKE_SOURCE_DIR}/include"
)

#target_compile_options(${TARGET_NAME}
#  PRIVATE
#    -Wall -Wextra -Wcast-qual -Wconversion -Weffc++ -Wold-style-cast -Wpedantic -Wshadow-all
#)
