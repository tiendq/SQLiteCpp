#include <sqlite3.h>
#include <SQLiteCpp/Exception.h>

using namespace std;

namespace SQLite {

Exception::Exception(const std::string& aErrorMessage) :
    runtime_error{aErrorMessage},
    mErrcode(-1), // 0 would be SQLITE_OK, which doesn't make sense
    mExtendedErrcode(-1) {
}

Exception::Exception(const std::string& aErrorMessage, int code) :
    runtime_error{aErrorMessage},
    mErrcode(code),
    mExtendedErrcode(-1)
{
}

Exception::Exception(sqlite3* apSQLite) :
    runtime_error{sqlite3_errmsg(apSQLite)},
    mErrcode(sqlite3_errcode(apSQLite)),
    mExtendedErrcode(sqlite3_extended_errcode(apSQLite))
{
}

Exception::Exception(sqlite3* apSQLite, int code) :
    runtime_error(sqlite3_errmsg(apSQLite)),
    mErrcode(code),
    mExtendedErrcode(sqlite3_extended_errcode(apSQLite))
{
}

// Return a string, solely based on the error code
const char* Exception::what() const noexcept {
  return sqlite3_errstr(mErrcode);
}
} // SQLite
