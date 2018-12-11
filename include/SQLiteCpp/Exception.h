#pragma once

#include <exception>
#include <string>

// Forward declaration to avoid inclusion of <sqlite3.h> in a header
struct sqlite3;

namespace SQLite {

/**
 * @brief Encapsulation of the error message from SQLite3, based on std::runtime_error.
 */
class Exception : public std::runtime_error {
public:
  /**
   * @brief Encapsulation of the error message from SQLite3, based on std::runtime_error.
   *
   * @param[in] message The string message describing the SQLite error
   */
  explicit Exception(std::string const &message);

  /**
   * @brief Encapsulation of the error message from SQLite3, based on std::runtime_error.
   *
   * @param[in] sqlite The SQLite object, to obtain detailed error messages from.
   */
  explicit Exception(sqlite3* sqlite);

  inline int code() const noexcept {
    return m_code;
  }

  inline int extendedCode() const noexcept {
    return m_extendedCode;
  }

private:
  int m_code;
  int m_extendedCode;
};

} // SQLite
