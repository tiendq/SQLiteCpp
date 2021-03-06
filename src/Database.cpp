#include <fstream>
#include <string>
#include <sqlite3.h>
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Assertion.h>
#include <SQLiteCpp/Exception.h>

#ifndef SQLITE_DETERMINISTIC
#define SQLITE_DETERMINISTIC 0x800
#endif // SQLITE_DETERMINISTIC

using namespace std;

namespace SQLite {

const int   OPEN_READONLY   = SQLITE_OPEN_READONLY;
const int   OPEN_READWRITE  = SQLITE_OPEN_READWRITE;
const int   OPEN_CREATE     = SQLITE_OPEN_CREATE;
const int   OPEN_URI        = SQLITE_OPEN_URI;
const int   OK              = SQLITE_OK;
const char* VERSION         = SQLITE_VERSION;
const int   VERSION_NUMBER  = SQLITE_VERSION_NUMBER;

// Return SQLite version string using runtime call to the compiled library
string getLibVersion() noexcept {
  return sqlite3_libversion();
}

// Return SQLite version number using runtime call to the compiled library
int getLibVersionNumber() noexcept {
  return sqlite3_libversion_number();
}

// Open the provided database UTF-8 filename with SQLite::OPEN_xxx provided flags.
Database::Database(const string& aFilename,
                   const int          aFlags,
                   const int          aBusyTimeoutMs /* = 0 */,
                   const string& aVfs           /* = "" */) :
    mpSQLite{nullptr},
    mFilename{aFilename}
{
  open(aFilename, aFlags, aBusyTimeoutMs, aVfs);
}

// Open a temporary in-memory database by default, use SQLite::TEMPORARY to open a temporary on-disk database.
Database::Database(string const &fileName) : mpSQLite(nullptr), mFilename(fileName) {
  SQLITECPP_ASSERT(MEMORY == fileName || TEMPORARY == fileName, "Default access mode OPEN_READWRITE | OPEN_CREATE is only used for temporary databases");
  open(fileName, OPEN_READWRITE | OPEN_CREATE, 0, "");
}

// Close the SQLite database connection.
Database::~Database() {
  int result = sqlite3_close_v2(mpSQLite);
  SQLITECPP_ASSERT(SQLITE_OK == result, sqlite3_errmsg(mpSQLite));
}

/**
 * @brief Set a busy handler that sleeps for a specified amount of time when a table is locked.
 *
 *  This is useful in multithreaded program to handle case where a table is locked for writting by a thread.
 *  Any other thread cannot access the table and will receive a SQLITE_BUSY error:
 *  setting a timeout will wait and retry up to the time specified before returning this SQLITE_BUSY error.
 *  Reading the value of timeout for current connection can be done with SQL query "PRAGMA busy_timeout;".
 *  Default busy timeout is 0ms.
 *
 * @param[in] aBusyTimeoutMs    Amount of milliseconds to wait before returning SQLITE_BUSY
 *
 * @throw SQLite::Exception in case of error
 */
void Database::setBusyTimeout(const int aBusyTimeoutMs) {
  const int ret = sqlite3_busy_timeout(mpSQLite, aBusyTimeoutMs);
  check(ret);
}

// Shortcut to execute one or multiple SQL statements without results (UPDATE, INSERT, ALTER, COMMIT, CREATE...).
int Database::exec(string const &queries) {
  const int ret = sqlite3_exec(mpSQLite, queries.c_str(), nullptr, nullptr, nullptr);
  check(ret);

  // Return the number of rows modified by those SQL statements (INSERT, UPDATE or DELETE only)
  return sqlite3_changes(mpSQLite);
}

// Shortcut to execute a one step query and fetch the first column of the result.
// WARNING: Be very careful with this dangerous method: you have to
// make a COPY OF THE result, else it will be destroy before the next line
// (when the underlying temporary Statement and Column objects are destroyed)
// this is an issue only for pointer type result (ie. char* and blob)
// (use the Column copy-constructor)
Column Database::execAndGet(string const &query) {
  Statement statement(*this, query);
  (void)statement.executeStep(); // Can return false if no result, which will throw next line in getColumn()
  return statement.getColumn(0);
}

// TODO: hasTable
// Shortcut to test if a table exists.
bool Database::tableExists(string const &tableName) {
  Statement query(*this, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name=?");
  query.bind(1, tableName);
  (void)query.executeStep(); // Cannot return false, as the above query always return a result
  return (1 == query.getColumn(0).getInt());
}

// Get the rowid of the most recent successful INSERT into the database from the current connection.
long long Database::getLastInsertRowid() const noexcept {
  return sqlite3_last_insert_rowid(mpSQLite);
}

// Get total number of rows modified by all INSERT, UPDATE or DELETE statement since connection.
int Database::getTotalChanges() const noexcept {
  return sqlite3_total_changes(mpSQLite);
}

// Return the numeric result code for the most recent failed API call (if any).
int Database::getErrorCode() const noexcept {
  return sqlite3_errcode(mpSQLite);
}

// Return the extended numeric result code for the most recent failed API call (if any).
int Database::getExtendedErrorCode() const noexcept {
  return sqlite3_extended_errcode(mpSQLite);
}

// Return UTF-8 encoded English language explanation of the most recent failed API call (if any).
string const Database::getErrorMsg() const noexcept {
  return sqlite3_errmsg(mpSQLite);
}

// Attach a custom function to your sqlite database. Assumes UTF8 text representation.
// Parameter details can be found here: http://www.sqlite.org/c3ref/create_function.html
void Database::createFunction(string const &apFuncName,
                              int           aNbArg,
                              bool          abDeterministic,
                              void*         apApp,
                              void        (*apFunc)(sqlite3_context *, int, sqlite3_value **),
                              void        (*apStep)(sqlite3_context *, int, sqlite3_value **),
                              void        (*apFinal)(sqlite3_context *),   // NOLINT(readability/casting)
                              void        (*apDestroy)(void *))
{
  int TextRep = SQLITE_UTF8;
  // optimization if deterministic function (e.g. of nondeterministic function random())
  if (abDeterministic) {
      TextRep = TextRep|SQLITE_DETERMINISTIC;
  }
  const int ret = sqlite3_create_function_v2(mpSQLite, apFuncName.c_str(), aNbArg, TextRep,
                                              apApp, apFunc, apStep, apFinal, apDestroy);
  check(ret);
}

// Load an extension into the sqlite database. Only affects the current connection.
// Parameter details can be found here: http://www.sqlite.org/c3ref/load_extension.html
void Database::loadExtension(string const &apExtensionName, string const &apEntryPointName) {
#ifdef SQLITE_OMIT_LOAD_EXTENSION
  // Unused
  (void)apExtensionName;
  (void)apEntryPointName;

  throw runtime_error("sqlite extensions are disabled");
#else
#ifdef SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION // Since SQLite 3.13 (2016-05-18):
  // Security warning:
  // It is recommended that the SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION method be used to enable only this interface.
  // The use of the sqlite3_enable_load_extension() interface should be avoided to keep the SQL load_extension()
  // disabled and prevent SQL injections from giving attackers access to extension loading capabilities.
  // (NOTE: not using nullptr: cannot pass object of non-POD type 'std::__1::nullptr_t' through variadic function)
  int ret = sqlite3_db_config(mpSQLite, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, NULL); // NOTE: not using nullptr
#else
  int ret = sqlite3_enable_load_extension(mpSQLite, 1);
#endif
  check(ret);

  ret = sqlite3_load_extension(mpSQLite, apExtensionName.c_str(), apEntryPointName.c_str(), 0);
  check(ret);
#endif
}

// Set the key for the current sqlite database instance.
void Database::key(const string& aKey) const {
  int pass_len = static_cast<int>(aKey.length());
#ifdef SQLITE_HAS_CODEC
  if (pass_len > 0) {
    const int ret = sqlite3_key(mpSQLite, aKey.c_str(), pass_len);
    check(ret);
  }
#else // SQLITE_HAS_CODEC
  if (pass_len > 0) {
    const SQLite::Exception exception("No encryption support, recompile with SQLITE_HAS_CODEC to enable.");
    throw exception;
  }
#endif // SQLITE_HAS_CODEC
}

// Reset the key for the current sqlite database instance.
void Database::rekey(const string& aNewKey) const {
#ifdef SQLITE_HAS_CODEC
  int pass_len = aNewKey.length();
  if (pass_len > 0) {
      const int ret = sqlite3_rekey(mpSQLite, aNewKey.c_str(), pass_len);
      check(ret);
  } else {
      const int ret = sqlite3_rekey(mpSQLite, nullptr, 0);
      check(ret);
  }
#else // SQLITE_HAS_CODEC
  static_cast<void>(aNewKey); // silence unused parameter warning
  const SQLite::Exception exception("No encryption support, recompile with SQLITE_HAS_CODEC to enable.");
  throw exception;
#endif // SQLITE_HAS_CODEC
}

// Test if a file contains an unencrypted database.
bool Database::isUnencrypted(const std::string& aFilename)
{
    if (aFilename.length() > 0) {
        std::ifstream fileBuffer(aFilename.c_str(), std::ios::in | std::ios::binary);
        char header[16];
        if (fileBuffer.is_open()) {
            fileBuffer.seekg(0, std::ios::beg);
            fileBuffer.getline(header, 16);
            fileBuffer.close();
        } else {
            const SQLite::Exception exception("Error opening file: " + aFilename);
            throw exception;
        }
        return strncmp(header, "SQLite format 3\000", 16) == 0;
    }
    const SQLite::Exception exception("Could not open database, the aFilename parameter was empty.");
    throw exception;
}

int Database::open(string const &fileName, int const flags, int const busyTimeoutMs, string const &vfs) {
  int result = sqlite3_open_v2(fileName.c_str(), &mpSQLite, flags, vfs.empty() ? nullptr : vfs.c_str());

  if (SQLITE_OK == result) {
    if (busyTimeoutMs > 0)
      setBusyTimeout(busyTimeoutMs);

    return SQLITE_OK;
  } else {
    Exception exception(mpSQLite);

    // Whether or not an error occurs when it is opened, resources associated with
    // the database connection handle should be released by passing it to sqlite3_close()
    // when it is no longer required.
    sqlite3_close_v2(mpSQLite);
    throw exception;
  }
}

} // SQLite
