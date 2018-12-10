#include <sqlite3.h>
#include <SQLiteCpp/Backup.h>
#include <SQLiteCpp/Exception.h>

namespace SQLite {

// Initialize resource for SQLite database backup
Backup::Backup(Database&            aDestDatabase,
               const std::string&   aDestDatabaseName,
               Database&            aSrcDatabase,
               const std::string&   aSrcDatabaseName) :
  mpSQLiteBackup{nullptr}
{
  mpSQLiteBackup = sqlite3_backup_init(aDestDatabase.getHandle(),
                                          aDestDatabaseName.c_str(),
                                          aSrcDatabase.getHandle(),
                                          aSrcDatabaseName.c_str());

  // If an error occurs, the error code and message are attached to the destination database connection.
  if (nullptr == mpSQLiteBackup)
    throw Exception(aDestDatabase.getHandle());
}

Backup::~Backup() {
  if (nullptr != mpSQLiteBackup)
    sqlite3_backup_finish(mpSQLiteBackup);
}

// Execute backup step with a given number of source pages to be copied
int Backup::executeStep(const int aNumPage /* = -1 */) {
  const int res = sqlite3_backup_step(mpSQLiteBackup, aNumPage);
  if (SQLITE_OK != res && SQLITE_DONE != res && SQLITE_BUSY != res && SQLITE_LOCKED != res)
    throw SQLite::Exception(sqlite3_errstr(res), res);

  return res;
}

// Get the number of remaining source pages to be copied in this backup process
int Backup::getRemainingPageCount() {
  return sqlite3_backup_remaining(mpSQLiteBackup);
}

// Get the number of total source pages to be copied in this backup process
int Backup::getTotalPageCount() {
  return sqlite3_backup_pagecount(mpSQLiteBackup);
}

} // SQLite
