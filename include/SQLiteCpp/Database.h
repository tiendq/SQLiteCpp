#pragma once

#include <string>
#include <SQLiteCpp/Column.h>
#include <SQLiteCpp/Utils.h>

// Forward declarations to avoid inclusion of <sqlite3.h> in a header
struct sqlite3;
struct sqlite3_context;

#ifndef SQLITE_USE_LEGACY_STRUCT // Since SQLITE 3.19 (used by default since SQLiteCpp 2.1.0)
typedef struct sqlite3_value sqlite3_value;
#else // Before SQLite 3.19 (legacy struct forward declaration can be activated with CMake SQLITECPP_LEGACY_STRUCT var)
struct Mem;
typedef struct Mem sqlite3_value;
#endif

namespace SQLite {

// Those public constants enable most usages of SQLiteCpp without including <sqlite3.h>
// in the client application. Have same name with SQLITE_ constants.
extern const int OPEN_READONLY;
extern const int OPEN_READWRITE;
extern const int OPEN_CREATE;
extern const int OPEN_URI;
extern const int OK;
extern const char *VERSION;
extern const int VERSION_NUMBER;

// Private, temporary in-memory database.
const std::string MEMORY = ":memory:";

// Private, temporary on-disk database.
const std::string TEMPORARY = "";

/// Return SQLite version string using runtime call to the compiled library
std::string getLibVersion() noexcept;

/// Return SQLite version number using runtime call to the compiled library
int getLibVersionNumber() noexcept;

/**
 * @brief RAII management of a SQLite Database Connection.
 *
 * A Database object manage a list of all SQLite Statements associated with the
 * underlying SQLite 3 database connection.
 *
 * Resource Acquisition Is Initialization (RAII) means that the Database Connection
 * is opened in the constructor and closed in the destructor, so that there is
 * no need to worry about memory management or the validity of the underlying SQLite Connection.
 *
 * Thread-safety: a Database object shall not be shared by multiple threads, because :
 * 1) in the SQLite "Thread Safe" mode, "SQLite can be safely used by multiple threads
 *    provided that no single database connection is used simultaneously in two or more threads."
 * 2) the SQLite "Serialized" mode is not supported by SQLiteC++,
 *    because of the way it shares the underling SQLite precompiled statement
 *    in a custom shared pointer (See the inner class "Statement::Ptr").
 */
class Database {
  // Give Statement constructor access to the mpSQLite Connection Handle
  friend class Statement;

public:
  /**
   * @brief Open the provided database UTF-8 filename.
   *
   * Uses sqlite3_open_v2() with readonly default flag, which is the opposite behavior
   * of the old sqlite3_open() function (READWRITE+CREATE).
   * This makes sense if you want to use it on a readonly filesystem
   * or to prevent creation of a void file when a required file is missing.
   *
   * Exception is thrown in case of error, then the Database object is NOT constructed.
   *
   * @param[in] aFilename         UTF-8 path/uri to the database file ("filename" sqlite3 parameter)
   * @param[in] aFlags            SQLite::OPEN_READONLY/SQLite::OPEN_READWRITE/SQLite::OPEN_CREATE...
   * @param[in] aBusyTimeoutMs    Amount of milliseconds to wait before returning SQLITE_BUSY (see setBusyTimeout())
   * @param[in] aVfs              UTF-8 name of custom VFS to use, or empty string for sqlite3 default
   *
   * @throw SQLite::Exception in case of error
   */
  Database(const std::string& aFilename,
            const int          aFlags,
            const int          aBusyTimeoutMs  = 0,
            const std::string& aVfs            = "");

  Database(std::string const &fileName);

  /**
   * @brief Close the SQLite database connection.
   *
   * All SQLite statements must have been finalized before,
   * so all Statement objects must have been unregistered.
   *
   * @warning assert in case of error
   */
  ~Database();

  /**
   * @brief Set a busy handler that sleeps for a specified amount of time when a table is locked.
   *
   *  This is useful in multithreaded program to handle case where a table is locked for writing by a thread.
   * Any other thread cannot access the table and will receive a SQLITE_BUSY error:
   * setting a timeout will wait and retry up to the time specified before returning this SQLITE_BUSY error.
   *  Reading the value of timeout for current connection can be done with SQL query "PRAGMA busy_timeout;".
   *  Default busy timeout is 0ms.
   *
   * @param[in] aBusyTimeoutMs    Amount of milliseconds to wait before returning SQLITE_BUSY
   *
   * @throw SQLite::Exception in case of error
   */
  void setBusyTimeout(const int aBusyTimeoutMs);

  /**
   * @brief Shortcut to execute one or multiple statements without results.
   *
   *  This is useful for any kind of statements other than the Data Query Language (DQL) "SELECT" :
   *  - Data Manipulation Lganguage (DML) statements "INSERT", "UPDATE" and "DELETE"
   *  - Data Definition Language (DDL) statements "CREATE", "ALTER" and "DROP"
   *  - Data Control Language (DCL) statements "GRANT", "REVOKE", "COMMIT" and "ROLLBACK"
   *
   * @see Statement::exec() to handle precompiled statements (for better performances) without results
   * @see Statement::executeStep() to handle "SELECT" queries with results
   *
   * @param[in] aQueries  one or multiple UTF-8 encoded, semicolon-separate SQL statements
   *
   * @return number of rows modified by the *last* INSERT, UPDATE or DELETE statement (beware of multiple statements)
   * @warning undefined for CREATE or DROP table: returns the value of a previous INSERT, UPDATE or DELETE statement.
   *
   * @throw SQLite::Exception in case of error
   */
  int exec(std::string const &queries);

  /**
   * @brief Shortcut to execute a one step query and fetch the first column of the result.
   *
   *  This is a shortcut to execute a simple statement with a single result.
   * This should be used only for non reusable queries (else you should use a Statement with bind()).
   * This should be used only for queries with expected results (else an exception is fired).
   *
   * @warning WARNING: Be very careful with this dangerous method: you have to
   *          make a COPY OF THE result, else it will be destroy before the next line
   *          (when the underlying temporary Statement and Column objects are destroyed)
   *
   * @see also Statement class for handling queries with multiple results
   *
   * @param[in] apQuery  an UTF-8 encoded SQL query
   *
   * @return a temporary Column object with the first column of result.
   *
   * @throw SQLite::Exception in case of error
   */
  Column execAndGet(std::string const &query);

  /**
   * @brief Shortcut to test if a table exists.
   *
   *  Table names are case sensitive.
   *
   * @param[in] apTableName an UTF-8 encoded case sensitive Table name
   *
   * @return true if the table exists.
   *
   * @throw SQLite::Exception in case of error
   */
  bool tableExists(std::string const &tableName);

  /**
   * @brief Get the rowid of the most recent successful INSERT into the database from the current connection.
   *
   *  Each entry in an SQLite table always has a unique 64-bit signed integer key called the rowid.
   * If the table has a column of type INTEGER PRIMARY KEY, then it is an alias for the rowid.
   *
   * @return Rowid of the most recent successful INSERT into the database, or 0 if there was none.
   */
  long long getLastInsertRowid() const noexcept;

  /// Get total number of rows modified by all INSERT, UPDATE or DELETE statement since connection (not DROP table).
  int getTotalChanges() const noexcept; // nothrow

  /// Return the numeric result code for the most recent failed API call (if any).
  int getErrorCode() const noexcept; // nothrow
  /// Return the extended numeric result code for the most recent failed API call (if any).
  int getExtendedErrorCode() const noexcept; // nothrow
  /// Return UTF-8 encoded English language explanation of the most recent failed API call (if any).
  std::string const getErrorMsg() const noexcept; // nothrow

  /// Return the filename used to open the database.
  std::string const& getFilename() const noexcept {
    return mFilename;
  }

  /**
   * @brief Return raw pointer to SQLite Database Connection Handle.
   *
   * This is often needed to mix this wrapper with other libraries or for advance usage not supported by SQLiteCpp.
   */
  inline sqlite3* getHandle() const noexcept // nothrow
  {
    return mpSQLite;
  }

  /**
   * @brief Create or redefine a SQL function or aggregate in the sqlite database.
   *
   *  This is the equivalent of the sqlite3_create_function_v2 command.
   * @see http://www.sqlite.org/c3ref/create_function.html
   *
   * @note UTF-8 text encoding assumed.
   *
   * @param[in] aFuncName     Name of the SQL function to be created or redefined
   * @param[in] aNbArg        Number of arguments in the function
   * @param[in] abDeterministic Optimize for deterministic functions (most are). A random number generator is not.
   * @param[in] apApp         Arbitrary pointer of user data, accessible with sqlite3_user_data().
   * @param[in] apFunc        Pointer to a C-function to implement a scalar SQL function (apStep & apFinal nullptr)
   * @param[in] apStep        Pointer to a C-function to implement an aggregate SQL function (apFunc nullptr)
   * @param[in] apFinal       Pointer to a C-function to implement an aggregate SQL function (apFunc nullptr)
   * @param[in] apDestroy     If not nullptr, then it is the destructor for the application data pointer.
   *
   * @throw SQLite::Exception in case of error
   */
  void createFunction(const std::string&   aFuncName,
                              int                  aNbArg,
                              bool                 abDeterministic,
                              void*                apApp,
                              void               (*apFunc)(sqlite3_context *, int, sqlite3_value **),
                              void               (*apStep)(sqlite3_context *, int, sqlite3_value **),
                              void               (*apFinal)(sqlite3_context *), // NOLINT(readability/casting)
                              void               (*apDestroy)(void *));

  /**
   * @brief Load a module into the current sqlite database instance.
   *
   *  This is the equivalent of the sqlite3_load_extension call, but additionally enables
   *  module loading support prior to loading the requested module.
   *
   * @see http://www.sqlite.org/c3ref/load_extension.html
   *
   * @note UTF-8 text encoding assumed.
   *
   * @param[in] apExtensionName   Name of the shared library containing extension
   * @param[in] apEntryPointName  Name of the entry point (nullptr to let sqlite work it out)
   *
   * @throw SQLite::Exception in case of error
   */
  void loadExtension(std::string const &apExtensionName, std::string const &apEntryPointName);

  /**
  * @brief Set the key for the current sqlite database instance.
  *
  *  This is the equivalent of the sqlite3_key call and should thus be called
  *  directly after opening the database.
  *  Open encrypted database -> call db.key("secret") -> database ready
  *
  * @param[in] aKey   Key to decode/encode the database
  *
  * @throw SQLite::Exception in case of error
  */
  void key(std::string const &aKey) const;

  /**
  * @brief Reset the key for the current sqlite database instance.
  *
  *  This is the equivalent of the sqlite3_rekey call and should thus be called
  *  after the database has been opened with a valid key. To decrypt a
  *  database, call this method with an empty string.
  *  Open normal database -> call db.rekey("secret") -> encrypted database, database ready
  *  Open encrypted database -> call db.key("secret") -> call db.rekey("newsecret") -> change key, database ready
  *  Open encrypted database -> call db.key("secret") -> call db.rekey("") -> decrypted database, database ready
  *
  * @param[in] aNewKey   New key to encode the database
  *
  * @throw SQLite::Exception in case of error
  */
  void rekey(const std::string& aNewKey) const;

  /**
  * @brief Test if a file contains an unencrypted database.
  *
  *  This is a simple test that reads the first bytes of a database file and
  *  compares them to the standard header for unencrypted databases. If the
  *  header does not match the standard string, we assume that we have an
  *  encrypted file.
  *
  * @param[in] aFilename path/uri to a file
  *
  * @return true if the database has the standard header.
  *
  * @throw SQLite::Exception in case of error
  */
  static bool isUnencrypted(const std::string& aFilename);

private:
  /// @{ Database must be non-copyable
  Database(Database const &db);
  Database& operator =(Database const &db);
  /// @}

  /**
   * @brief Check if aRet equal SQLITE_OK, else throw a SQLite::Exception with the SQLite error message
   */
  inline void check(const int aRet) const
  {
      if (SQLite::OK != aRet)
          throw SQLite::Exception(mpSQLite);
  }

  int open(std::string const &fileName, int const flags, int const busyTimeoutMs, std::string const &vfs);

  sqlite3*    mpSQLite;   ///< Pointer to a SQLite database connection handle
  std::string mFilename;  ///< UTF-8 file name used to open the database
};
} // SQLite
