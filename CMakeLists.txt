cmake_minimum_required(VERSION 3.12)
project(SQLiteCpp)

message (STATUS "CMake version: ${CMAKE_VERSION}")

set(SQLITECPP_INTERNAL_SQLITE OFF CACHE BOOL "Build internal SQLite3")
set(SQLITECPP_RUN_DOXYGEN OFF CACHE BOOL "Build documentation with Doxygen")
set(SQLITECPP_BUILD_EXAMPLES OFF CACHE BOOL "Build examples")
set(SQLITECPP_BUILD_TESTS OFF CACHE BOOL "Build unit tests")

if(WIN32)
  set(DEV_NULL "NUL")
else()
  set(DEV_NULL "/dev/null")
endif()

if(MSVC)
  # disable Visual Studio warnings for fopen() used in the example
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)

  # Flags for linking with multithread static C++ runtime, required by googletest
  if(SQLITECPP_BUILD_TESTS)
    message(STATUS "Linking against multithread static C++ runtime for unit tests with googletest")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  endif()
else()
  # Useful compile flags and extra warnings
  add_compile_options(-fstack-protector -Wall -Wextra -Wpedantic -Wno-long-long -Wswitch-enum -Wshadow -Winline)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++0x-compat")

  if(CMAKE_COMPILER_IS_GNUCXX)
    if(SQLITECPP_USE_GCOV AND CMAKE_COMPILER_IS_GNUCXX)
      if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message (STATUS "Using GCov instrumentation")
      else()
        message (WARNING "GCov instrumentation works best in Debug mode")
      endif()

      # NOTE would be usefull but not working with current google test and gcc 4.8 -fkeep-inline-functions
      add_compile_options (-coverage)
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -coverage")
    endif()
  endif()
endif()

# Print CXX compiler information
message(STATUS "CMAKE_CXX_COMPILER '${CMAKE_CXX_COMPILER}' '${CMAKE_CXX_COMPILER_ID}' '${CMAKE_CXX_COMPILER_VERSION}'")
message(STATUS "CMAKE_CXX_FLAGS '${CMAKE_CXX_FLAGS}'")

if(MSVC)
  message(STATUS "CMAKE_CXX_FLAGS_DEBUG '${CMAKE_CXX_FLAGS_DEBUG}'")
  message(STATUS "CMAKE_CXX_FLAGS_RELEASE '${CMAKE_CXX_FLAGS_RELEASE}'")
  message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO '${CMAKE_CXX_FLAGS_RELWITHDEBINFO}'")
  message(STATUS "CMAKE_CXX_FLAGS_MINSIZEREL '${CMAKE_CXX_FLAGS_MINSIZEREL}'")
else()
  if(CMAKE_BUILD_TYPE STREQUAL Debug)
    message(STATUS "CMAKE_CXX_FLAGS_DEBUG '${CMAKE_CXX_FLAGS_DEBUG}'")
  elseif(CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
    message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO '${CMAKE_CXX_FLAGS_RELWITHDEBINFO}'")
  elseif(CMAKE_BUILD_TYPE STREQUAL MinSizeRel)
    message(STATUS "CMAKE_CXX_FLAGS_MINSIZEREL '${CMAKE_CXX_FLAGS_MINSIZEREL}'")
  else()
    message(STATUS "CMAKE_CXX_FLAGS_RELEASE '${CMAKE_CXX_FLAGS_RELEASE}'")
  endif()
endif()

# Options relative to SQLite and SQLiteC++ functions
option(SQLITE_ENABLE_COLUMN_METADATA "Enable Column::getColumnOriginName(). Require support from sqlite3 library." ON)

if(SQLITE_ENABLE_COLUMN_METADATA)
  # Enable the use of SQLite column metadata and Column::getColumnOriginName() method,
  # Require that the sqlite3 library is also compiled with this flag (default under Debian/Ubuntu, but not on Mac OS X).
  add_definitions(-DSQLITE_ENABLE_COLUMN_METADATA)
endif()

option(SQLITE_ENABLE_ASSERT_HANDLER "Enable the user defintion of a assertion_failed() handler." OFF)
if(SQLITE_ENABLE_ASSERT_HANDLER)
  # Enable the user defintion of a assertion_failed() handler (default to false, easier to handler for begginers).
  add_definitions(-DSQLITECPP_ENABLE_ASSERT_HANDLER)
endif()

option(SQLITE_USE_LEGACY_STRUCT "Fallback to forward declaration of legacy struct sqlite3_value (pre SQLite 3.19)" OFF)
if(SQLITE_USE_LEGACY_STRUCT)
  # Force forward declaration of legacy struct sqlite3_value (pre SQLite 3.19)
  add_definitions(-DSQLITE_USE_LEGACY_STRUCT)
endif()

## Build the C++ Wrapper ##
set(SQLITECPP_SRC
  src/Backup.cpp
  src/Column.cpp
  src/Database.cpp
  src/Exception.cpp
  src/Statement.cpp
  src/Transaction.cpp
)
source_group(src FILES ${SQLITECPP_SRC})

set(SQLITECPP_INC
  include/SQLiteCpp/SQLiteCpp.h
  include/SQLiteCpp/Assertion.h
  include/SQLiteCpp/Backup.h
  include/SQLiteCpp/Column.h
  include/SQLiteCpp/Database.h
  include/SQLiteCpp/Exception.h
  include/SQLiteCpp/Statement.h
  include/SQLiteCpp/Transaction.h
  include/SQLiteCpp/Utils.h
  include/SQLiteCpp/VariadicBind.h
)
source_group(include FILES ${SQLITECPP_INC})

set(SQLITECPP_TESTS
  src/tests/Column_test.cpp
  src/tests/Database_test.cpp
  src/tests/Statement_test.cpp
  src/tests/Backup_test.cpp
  src/tests/Transaction_test.cpp
  src/tests/VariadicBind_test.cpp
  src/tests/Exception_test.cpp
)
source_group(tests FILES ${SQLITECPP_TESTS})

set(SQLITECPP_EXAMPLES
  src/examples/example1/main.cpp
)
source_group(examples FILES ${SQLITECPP_EXAMPLES})

set(SQLITECPP_DOC
  README.md
  LICENSE.txt
  CHANGELOG.md
)
source_group(docs FILES ${SQLITECPP_DOC})

set(SQLITECPP_SCRIPT
  .travis.yml
  appveyor.yml
  Doxyfile
  FindSQLiteCpp.cmake
)
source_group(scripts FILES ${SQLITECPP_SCRIPT})

set(TARGET_NAME SQLiteCpp)

add_library(${TARGET_NAME} ${SQLITECPP_SRC} ${SQLITECPP_INC} ${SQLITECPP_DOC} ${SQLITECPP_SCRIPT})
target_include_directories(${TARGET_NAME} PRIVATE include)

# make the sqlite3 library part of the interface of the SQLiteCpp wrapper itself (the client app does not need to link to sqlite3)
# PR https://github.com/SRombauts/SQLiteCpp/pull/111 "linked SQLiteCpp to sqlite3" commented out since it breacks install step from PR #118
#target_link_libraries(SQLiteCpp PUBLIC sqlite3)

if (UNIX AND (CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
  set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

# Allow the library to be installed via "make install" and found with "find_package"
install(TARGETS ${TARGET_NAME}
  EXPORT ${PROJECT_NAME}Config
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  COMPONENT libraries
)

target_include_directories(${TARGET_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/>)
install(DIRECTORY include/ DESTINATION include COMPONENT headers FILES_MATCHING REGEX ".*\\.(hpp|h)$")
install(EXPORT ${PROJECT_NAME}Config DESTINATION lib/cmake/${PROJECT_NAME})

## Build provided copy of SQLite3 C library ##

# TODO
#find_package(sqlite3)
#if(sqlite3_VERSION VERSION_LESS "3.19")
#    set_target_properties(SQLiteCpp PROPERTIES COMPILE_FLAGS "-DSQLITECPP_HAS_MEM_STRUCT")
#endif()

# option(SQLITECPP_INTERNAL_SQLITE "Add the internal SQLite3 source to the project." OFF)
if(SQLITECPP_INTERNAL_SQLITE)
  # build the SQLite3 C library (for ease of use/compatibility) versus Linux sqlite3-dev package
  add_subdirectory(libs/sqlite3)
  target_include_directories(${TARGET_NAME} PRIVATE libs/sqlite3)
endif()

# option(SQLITECPP_RUN_DOXYGEN "Run Doxygen C++ documentation tool." OFF)
if(SQLITECPP_RUN_DOXYGEN)
  find_package(Doxygen)

  if(DOXYGEN_FOUND)
    add_custom_target(SQLiteCpp_doxygen
      ALL
      COMMAND doxygen Doxyfile > ${DEV_NULL}
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
  endif()
else()
  message(STATUS "SQLITECPP_RUN_DOXYGEN OFF")
endif()

# option(SQLITECPP_BUILD_EXAMPLES "Build examples." OFF)
if(SQLITECPP_BUILD_EXAMPLES)
  add_executable(SQLiteCpp_example1 ${SQLITECPP_EXAMPLES})
  target_link_libraries(SQLiteCpp_example1 SQLiteCpp sqlite3)

  # Link target with pthread and dl for linux
  if(UNIX)
    target_link_libraries(SQLiteCpp_example1 pthread)
    if(NOT APPLE)
      target_link_libraries(SQLiteCpp_example1 dl)
    endif()
  elseif(MSYS OR MINGW)
    target_link_libraries(SQLiteCpp_example1 ssp)
  endif()
else()
  message(STATUS "SQLITECPP_BUILD_EXAMPLES OFF")
endif()

# option(SQLITECPP_BUILD_TESTS "Build and run tests." OFF)
if (SQLITECPP_BUILD_TESTS)
  # deactivate some warnings for compiling the gtest library
  if (NOT MSVC)
    add_compile_options(-Wno-variadic-macros -Wno-long-long -Wno-switch-enum -Wno-float-equal -Wno-conversion-null -Wno-switch-default -Wno-pedantic)
  endif()

  # add the subdirectory containing the CMakeLists.txt for the gtest library
  # TODO: under Linux, uses libgtest-dev if found
  if(NOT EXISTS "${PROJECT_SOURCE_DIR}/libs/googletest/CMakeLists.txt")
    message(FATAL_ERROR "Missing 'googletest' submodule! Either use 'git init submodule' and 'git update submodule' to get googletest according to the README, or deactivate unit tests with -DSQLITECPP_BUILD_TESTS=OFF")
  endif()

  add_subdirectory(libs/googletest)

  add_executable(SQLiteCpp_tests ${SQLITECPP_TESTS})
  target_compile_features(SQLiteCpp_tests PRIVATE cxx_std_17)
  target_include_directories(SQLiteCpp_tests PRIVATE libs/googletest/googletest/include)
  target_link_libraries(SQLiteCpp_tests gtest_main SQLiteCpp sqlite3)

  if(UNIX AND NOT APPLE)
    target_link_libraries(SQLiteCpp_tests dl)
  endif()

  enable_testing()
  add_test(UnitTests SQLiteCpp_tests)

  if(SQLITECPP_BUILD_EXAMPLES)
    add_test(Example1Run SQLiteCpp_example1)
  endif()
else()
  message(STATUS "SQLITECPP_BUILD_TESTS OFF")
endif()
