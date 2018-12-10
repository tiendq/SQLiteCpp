#pragma once

#include <string>
#include <SQLiteCpp/Database.h>

// Forward declaration to avoid inclusion of <sqlite3.h> in a header
struct sqlite3_backup;

namespace SQLite {

/**
 * A Backup object is used to backup a source database file to a destination database file
 * in a safe and online way.
 *
 * Thread-safety: a Backup object shall not be shared by multiple threads, because:
 * 1) in the SQLite "Thread Safe" mode, "SQLite can be safely used by multiple threads
 *    provided that no single database connection is used simultaneously in two or more threads."
 * 2) the SQLite "Serialized" mode is not supported by SQLiteC++,
 *    because of the way it shares the underling SQLite precompiled statement
 *    in a custom shared pointer (See the inner class "Statement::Ptr").
 */
class Backup {
public:
  /**
   * @brief Initialize a SQLite Backup object.
   *
   * Initialize a SQLite Backup object for source database and destination database.
   * The database name is "main" for the main database, "temp" for the temporary database,
   * or the name specified after the AS keyword in an ATTACH statement for an attached database.
   *
   * Exception is thrown in case of error, then the Backup object is NOT constructed.
   *
   * @param[in] aDestDatabase        Destination database connection
   * @param[in] aDestDatabaseName    Destination database name
   * @param[in] aSrcDatabase         Source database connection
   * @param[in] aSrcDatabaseName     Source database name
   *
   * @throw SQLite::Exception in case of error
   */
  Backup(Database&          aDestDatabase,
          const std::string& aDestDatabaseName,
          Database&          aSrcDatabase,
          const std::string& aSrcDatabaseName);

  ~Backup();

  /**
   * @brief Execute a step of backup with a given number of source pages to be copied
   *
   * Exception is thrown when SQLITE_IOERR_XXX, SQLITE_NOMEM, or SQLITE_READONLY is returned
   * in sqlite3_backup_step(). These errors are considered fatal, so there is no point
   * in retrying the call to executeStep().
   *
   * @param[in] aNumPage    The number of source pages to be copied, with a negative value meaning all remaining source pages
   *
   * @return SQLITE_OK/SQLITE_DONE/SQLITE_BUSY/SQLITE_LOCKED
   *
   * @throw SQLite::Exception in case of error
   */
  int executeStep(const int aNumPage = -1);

  /// Return the number of source pages still to be backed up as of the most recent call to executeStep().
  int getRemainingPageCount();

  /// Return the total number of pages in the source database as of the most recent call to executeStep().
  int getTotalPageCount();

private:
  /// @{ Backup must be non-copyable
  Backup(Backup const &);
  Backup& operator =(Backup const &);
  /// @}

private:
  sqlite3_backup* mpSQLiteBackup;   ///< Pointer to SQLite Database Backup Handle
};

} // SQLite
