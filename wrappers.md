http://www.sqlite.org/cvstrac/wiki?p=SqliteWrappers
http://stackoverflow.com/questions/120295/what-is-a-good-oo-c-wrapper-for-sqlite
http://stackoverflow.com/questions/818155/sqlite-alternatives-for-c


 - **sqdbcpp**: RAII design, no depandencies, UTF-8/UTF-16, new BSD license (http://code.google.com/p/sqdbcpp/)
 - **sqlite3pp**: uses boost, MIT License (http://code.google.com/p/sqlite3pp/)
 - **SQLite++**: uses boost build system, Boost License 1.0 (http://sqlitepp.berlios.de/)
 - **CppSQLite**: famous Code Project but old design, BSD License (http://www.codeproject.com/Articles/6343/CppSQLite-C-Wrapper-for-SQLite/)
 
 
**sqlite3cc**:
- http://ed.am/dev/sqlite3cc (and https://launchpad.net/sqlite3cc)
- (++) modern design, use RAII => can be a source of inspiration for me
- (++) very well documented, in code and with a very good informal presentation
- (+) is maintained (recent), initial release is 0.1.0, January 2012 (started in 2010)
- (+/-) uses boost (some more dependancies...)
- (-) a bit complicated : offer many way to do the same thing where I would prefer a clean choice
- (-) thus it does not impose RAII, as it is still possible to open or close a database outside constructor/destructor
- (---) LPGPL : for me, this is a stopper as I would like to be able to use it in commercial products
 
 => inspiration :
 - bind named parameters,
 - support for different transaction mode
 - comment on returning error code instead of exception that shall not be thrown when exepected (!?)
 - explain the noncopyable property for RAII design
