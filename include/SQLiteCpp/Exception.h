#pragma once

#include <exception>
#include <string>

// Forward declaration to avoid inclusion of <sqlite3.h> in a header
struct sqlite3;

/// Compatibility with non-clang compilers.
#ifndef __has_feature
    #define __has_feature(x) 0
#endif

// Detect whether the compiler supports C++11 noexcept exception specifications.
#if (  defined(__GNUC__) && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 7) || (__GNUC__ > 4)) \
    && defined(__GXX_EXPERIMENTAL_CXX0X__))
// GCC 4.7 and following have noexcept
#elif defined(__clang__) && __has_feature(cxx_noexcept)
// Clang 3.0 and above have noexcept
#elif defined(_MSC_VER) && _MSC_VER > 1800
// Visual Studio 2015 and above have noexcept
#else
    // Visual Studio 2013 does not support noexcept, and "throw()" is deprecated by C++11
    #define noexcept
#endif

namespace SQLite {

/**
 * @brief Encapsulation of the error message from SQLite3, based on std::runtime_error.
 */
class Exception : public std::runtime_error {
public:
  /**
   * @brief Encapsulation of the error message from SQLite3, based on std::runtime_error.
   *
   * @param[in] aErrorMessage The string message describing the SQLite error
   */
  explicit Exception(std::string const &aErrorMessage);

  /**
   * @brief Encapsulation of the error message from SQLite3, based on std::runtime_error.
   *
   * @param[in] aErrorMessage The string message describing the SQLite error
   * @param[in] ret           Return value from function call that failed.
   */
  Exception(std::string const &aErrorMessage, int code);

  /**
   * @brief Encapsulation of the error message from SQLite3, based on std::runtime_error.
   *
   * @param[in] apSQLite The SQLite object, to obtain detailed error messages from.
   */
  explicit Exception(sqlite3* apSQLite);

  /**
   * @brief Encapsulation of the error message from SQLite3, based on std::runtime_error.
   *
   * @param[in] apSQLite  The SQLite object, to obtain detailed error messages from.
   * @param[in] ret       Return value from function call that failed.
   */
  Exception(sqlite3* apSQLite, int code);

  /// Return the result code (if any, otherwise -1).
  inline int code() const noexcept {
    return mErrcode;
  }

  /// Return the extended numeric result code (if any, otherwise -1).
  inline int extendedCode() const noexcept {
    return mExtendedErrcode;
  }

  /// Return a string, solely based on the error code
  virtual const char* what() const noexcept;

private:
  int mErrcode;         ///< Error code value
  int mExtendedErrcode; ///< Detailed error code if any
};
} // SQLite
