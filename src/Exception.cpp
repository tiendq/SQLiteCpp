#include <sqlite3.h>
#include <SQLiteCpp/Exception.h>

using namespace std;

namespace SQLite {

Exception::Exception(string const &message) :
  runtime_error{message},
  m_code{SQLITE_ERROR},
  m_extendedCode{SQLITE_ERROR}
{
}

Exception::Exception(sqlite3* sqlite) :
  runtime_error{sqlite3_errmsg(sqlite)},
  m_code{sqlite3_errcode(sqlite)},
  m_extendedCode{sqlite3_extended_errcode(sqlite)}
{
}

} // SQLite
