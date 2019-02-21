SQLiteC++
---------

SQLiteC++ (SQLiteCpp) is an easy to use modern C++ wrapper for SQLite3 library. It offers an encapsulation around the native C APIs of SQLite, with a few intuitive and well documented C++ classes.

[SQLite](http://www.sqlite.org/about.html) is a C library that implements a serverless transactional SQL database engine. It is the most widely deployed SQL database engine in the world. All of the code and documentation in SQLite has been dedicated to the public domain by the authors.

## The goals of SQLiteC++

- to offer the best of the existing simple C++ SQLite wrappers
- to be elegantly written with good C++ design, STL, exceptions and RAII idiom
- to keep dependencies to a minimum (STL and SQLite3)
- to be portable
- to be light and fast
- to be thread-safe only as much as SQLite [multi-thread mode](https://www.sqlite.org/threadsafe.html)
- to have a good unit test coverage
- to use API names sticking with those of the SQLite library
- to be well documented with Doxygen tags, and with some good examples
- to be well maintained
- to use a permissive MIT license, similar to BSD or Boost, for proprietary/commercial usage

It is designed using the [RAII](http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization) idiom, and throwing exceptions in case of SQLite errors (exept in destructors, where `assert()` are used instead). Each SQLiteC++ object must be constructed with a valid SQLite database connection, and then is always valid until destroyed.

## Dependencies

- an STL implementation (even an old one, like the one provided with VC6 should work)
- exception support (the class `Exception` inherits from `std::runtime_error`)
- the SQLite library (3.7.15 minimum from 2012-12-12) either by linking to it dynamicaly or statically (install the `libsqlite3-dev` package under Debian/Ubuntu/Mint Linux), or by adding its source file in your project code base (source code provided in `sqlite3` for Windows), with the [`SQLITE_ENABLE_COLUMN_METADATA`](http://www.sqlite.org/compile.html#enable_column_metadata) macro defined.

## Getting started

### Installation

To use this wrapper, you need to add the SQLiteC++ source files from the `src` directory in your project code base, and compile/link against the `sqlite3` library.

The easiest way to do this is to add the wrapper as a library.

The `CMakeLists.txt` file defining the static library is provided in the root directory, so you simply have to `add_subdirectory(SQLiteCpp)` to you main `CMakeLists.txt` and link to the `SQLiteCpp` wrapper library.

Example for Linux:

```cmake
add_executable(main src/main.cpp)
add_subdirectory(${PROJECT_SOURCE_DIR}/libs/SQLiteCpp)

target_include_directories(main PRIVATE ${PROJECT_SOURCE_DIR}/libs/SQLiteCpp/include)
target_link_libraries(main SQLiteCpp sqlite3 dl)
```

This SQLiteCpp repository can be directly used as a Git submoldule in your main repository, see [SQLiteCppExample](https://github.com/tiendq/SQLiteCppExample) for detail.

Under Debian/Ubuntu/Mint Linux, you can install the `libsqlite3-dev` package if you don't want to use the embedded `sqlite3` library.

### Building example and unit tests

Clone the repository then init and update submodule `googletest`.

```shell
git clone --recurse-submodules https://github.com/tiendq/SQLiteCpp.git
```

#### CMake and tests
A CMake configuration file is also provided for multiplatform support and testing.

```shell
mkdir build
cd bebug

cmake -DSQLITECPP_BUILD_EXAMPLES=ON -DSQLITECPP_BUILD_TESTS=ON ..
cmake --build .

# Build and run unit-tests (ie 'make test')
ctest --output-on-failure
```

#### CMake options

* For more options on customizing the build, see the [CMakeLists.txt](https://github.com/tiendq/SQLiteCpp/blob/master/CMakeLists.txt) file.

#### Troubleshooting

Under Linux, if you get muliple linker errors like `undefined reference to sqlite3_xxx`,
it's that you lack the `sqlite3` library: install the `libsqlite3-dev` package.

If you get a single linker error `Column.cpp: undefined reference to sqlite3_column_origin_name`,
it's that your `sqlite3` library was not compiled with the `SQLITE_ENABLE_COLUMN_METADATA` macro defined.
You can either recompile it yourself (seek help online) or you can comment out the following line in `include/SQLiteCpp/Column.h`:

```c++
#define SQLITE_ENABLE_COLUMN_METADATA
```

### Thread-safety

SQLite supports three modes of thread safety, as described in [Multi-threaded Programs and SQLite](https://www.sqlite.org/threadsafe.html).

This SQLiteC++ does not add any locks (no mutexes) nor any other thread-safety mechanism
above the SQLite library itself, by design, for lightness and speed.

Thus, SQLiteC++ naturally supports the multi-thread mode of SQLite:

> In this mode, SQLite can be safely used by multiple threads provided that no single database connection is used simultaneously in two or more threads.

But SQLiteC++ does not support the fully thread-safe "Serialized" mode of SQLite, because of the way it shares the underlying SQLite precompiled statement in a custom shared pointer (see class `Statement::Ptr`).

### Examples

This example sample demonstrates how to query a database and get results.

```c++
try
{
    // Open a database file
    SQLite::Database    db("example.db3");

    // Compile a SQL query, containing one parameter (index 1)
    SQLite::Statement   query(db, "SELECT * FROM test WHERE size > ?");

    // Bind the integer value 6 to the first parameter of the SQL query
    query.bind(1, 6);

    // Loop to execute the query step by step, to get rows of result
    while (query.executeStep())
    {
        // Demonstrate how to get some typed column value
        int         id      = query.getColumn(0);
        const char* value   = query.getColumn(1);
        int         size    = query.getColumn(2);

        std::cout << "row: " << id << ", " << value << ", " << size << std::endl;
    }
}
catch (std::exception& e)
{
    std::cout << "exception: " << e.what() << std::endl;
}
```

### How to handle assertion in SQLiteC++
[Don't throw exceptions in destructors!](https://isocpp.org/wiki/faq/exceptions#dtors-shouldnt-throw), so SQLiteC++ uses `SQLITECPP_ASSERT()` to check for errors in destructors. If you don't want `assert()` to be called, you have to enable and define an assert handler as shown below, and by setting the flag `SQLITECPP_ENABLE_ASSERT_HANDLER` when compiling the library.

```c++
#ifdef SQLITECPP_ENABLE_ASSERT_HANDLER
namespace SQLite
{
/// Definition of the custom assertion handler, enabled when SQLITECPP_ENABLE_ASSERT_HANDLER is defined.
void assertion_failed(const char* apFile, const long apLine, const char* apFunc, const char* apExpr, const char* apMsg)
{
    // Print a message to the standard error output stream, and abort the program.
    std::cerr << apFile << ":" << apLine << ":" << " error: assertion failed (" << apExpr << ") in " << apFunc << "() with message \"" << apMsg << "\"\n";
    std::abort();
}
}
#endif // SQLITECPP_ENABLE_ASSERT_HANDLER
```

### Coding guidelines

The source code use the `CamelCase` naming style variant where:

- Type names (class, struct, typedef, enums...) begin with a capital letter e.g. `Database`
- Files are named like the class they contain e.g. `Database.cpp`
- Function and variable names begin with a lower case letter e.g. `bindNoCopy`
- Member variables begin with 'm_' e.g. `m_fileName`
- Each file, class, method and member variable is documented using Doxygen tags

### History

This repo is originally forked from [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp).
