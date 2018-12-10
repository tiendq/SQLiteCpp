#pragma once

#include <cassert>

/**
 * SQLITECPP_ASSERT SQLITECPP_ASSERT() is used in destructors, where exceptions shall not be thrown
 *
 * Define SQLITECPP_ENABLE_ASSERT_HANDLER at the project level
 * and define a SQLite::assertion_failed() assertion handler
 * to tell SQLiteC++ to use it instead of assert() when an assertion fail.
*/
#ifdef SQLITECPP_ENABLE_ASSERT_HANDLER

// If an assert handler is provided by user code, use it instead of assert()
namespace SQLite {
    // declaration of the assert handler to define in user code
    void assertion_failed(const char* apFile, const long apLine, const char* apFunc,
                          const char* apExpr, const char* apMsg);

#ifdef _MSC_VER
    #define __func__ __FUNCTION__
#endif
// call the assert handler provided by user code
#define SQLITECPP_ASSERT(expression, message) \
    if (!(expression))  SQLite::assertion_failed(__FILE__, __LINE__, __func__, #expression, message)
} // SQLite

#else

// If no assert handler provided by user code, use standard assert()
#define SQLITECPP_ASSERT(expression, message) assert(expression && message)

#endif // SQLITECPP_ENABLE_ASSERT_HANDLER
