add_executable(Database Database_test.cpp)

# make the sqlite3 library part of the interface of the SQLiteCpp wrapper itself (the client app does not need to link to sqlite3)
# PR https://github.com/SRombauts/SQLiteCpp/pull/111 "linked SQLiteCpp to sqlite3" commented out since it breacks install step from PR #118
#target_link_libraries(SQLiteCpp PUBLIC sqlite3)

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
  set(EXAMPLE_NAME SQLiteCpp_example1)
  add_executable(${EXAMPLE_NAME} ${SQLITECPP_EXAMPLES})
  target_compile_features(${EXAMPLE_NAME} PRIVATE cxx_std_17)
  target_link_libraries(${EXAMPLE_NAME} SQLiteCpp sqlite3)

  # Link target with pthread and dl for linux
  if(UNIX)
    target_link_libraries(${EXAMPLE_NAME} pthread)
    if(NOT APPLE)
      target_link_libraries(${EXAMPLE_NAME} dl)
    endif()
  elseif(MSYS OR MINGW)
    target_link_libraries(${EXAMPLE_NAME} ssp)
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
