#include <iostream>
#include <fstream>
#include <string>
#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

using namespace std;

#ifdef SQLITECPP_ENABLE_ASSERT_HANDLER
namespace SQLite {

void assertion_failed(const char* apFile, const long apLine, const char* apFunc, const char* apExpr, const char* apMsg) {
  std::cerr << apFile << ":" << apLine << ":" << " error: assertion failed (" << apExpr << ") in " << apFunc << "() with message \"" << apMsg << "\"\n";
  std::abort();
}

} // SQLite
#endif // SQLITECPP_ENABLE_ASSERT_HANDLER

string getFullPath(string const &fileName) {
  string filePath(__FILE__);
  return filePath.substr(0, filePath.length() - string("main.cpp").length()) + fileName;
}

string const dbFileName = getFullPath("example.db3");
string const logoFileName = getFullPath("logo.png");

class Example {
public:
  // Constructor
  Example() : m_db(dbFileName), m_query(m_db, "SELECT * FROM test WHERE weight > :min_weight") {
  }

  /// List the rows where the "weight" column is greater than the provided aParamValue
  void listGreaterThan (int const weight) {
    // Bind the integer value provided to the first parameter of the SQL query
    m_query.bind(":min_weight", weight); // same as mQuery.bind(1, weight);

    // Loop to execute the query step by step, to get one a row of results at a time
    while (m_query.executeStep())
      cout << "row (" << m_query.getColumn(0) << ", \"" << m_query.getColumn(1) << "\", " << m_query.getColumn(2) << ")\n";

    // Reset the query to be able to use it again later
    m_query.reset();
  }

private:
  SQLite::Database m_db;
  SQLite::Statement m_query;
};

// Exception dispatcher, https://isocpp.org/wiki/faq/exceptions#throw-without-an-object
int handleException() {
	try {
		throw;
	} catch (exception const &e) {
		cout << "SQLite exception: " << e.what() << '\n';
		return EXIT_FAILURE;
  }
}

int main() {
  // Using SQLITE_VERSION would require #include <sqlite3.h> which we want to avoid.
  // Use SQLite::VERSION if possible.
  cout << "SQLite3 version " << SQLite::VERSION << " (" << SQLite::getLibVersion() << ")" << '\n';
  cout << "SQLiteC++ version " << SQLITECPP_VERSION << '\n';

  // Very basic first example (1/7)
  try {
    // Open a database file in default readonly mode (SQLite::OPEN_READONLY)
    SQLite::Database db(dbFileName);
    cout << "SQLite database file '" << db.getFilename() << "' opened successfully\n";

    // Test if the 'test' table exists
    bool isTestExisted = db.tableExists("test");
    cout << "SQLite table 'test' exists: " << isTestExisted << '\n';

    // Get a single value result with an easy to use shortcut
    string value = db.execAndGet("SELECT value FROM test WHERE id=2");
    cout << "execAndGet: " << value << '\n';
  } catch (...) {
    return handleException();
  }

  // Simple select query - few variations (2/7)
  try {
      // Open a database file in default readonly mode (SQLite::OPEN_READONLY)
    SQLite::Database db(dbFileName);

    // Loop to get values of column by index, using auto cast to variable type
    // Compile a SQL query, containing one parameter (index 1)
    SQLite::Statement query(db, "SELECT id as test_id, value as test_val, weight as test_weight FROM test WHERE weight > ?");
    cout << "SQLite statement '" << query.getQuery() << "' compiled (" << query.getColumnCount () << " columns in the result)\n";

    // Bind the integer value 2 to the first parameter of the SQL query
    query.bind(1, 2);

    // Loop to execute the query step by step, to get one a row of results at a time
    // Demonstrates how to get some typed column value (and the equivalent explicit call)
    while (query.executeStep()) {
			const int id = query.getColumn(0); // = query.getColumn(0).getInt();
			const string value = query.getColumn(1); // = query.getColumn(1).getText();
			const int bytes = query.getColumn(1).size(); // .getColumn(1).getBytes();
			const double weight = query.getColumn(2); // = query.getColumn(2).getInt();
			cout << "row (" << id << ", \"" << value << "\"(" << bytes << ") " << weight << ")\n";
    }

    // Reset the query to use it again
    query.reset();
    cout << "SQLite statement '" << query.getQuery() << "' resetted (" << query.getColumnCount() << " columns in the result)\n";

    // Show how to get the aliased names of the result columns.
		// Get aliased column names (and original column names if possible)
    const string name0 = query.getColumnName(0);
    const string name1 = query.getColumnName(1);
    const string name2 = query.getColumnName(2);

		cout << "Aliased result [\"" << name0 << "\", \"" << name1  << "\", \"" << name2 << "\"]\n";

#ifdef SQLITE_ENABLE_COLUMN_METADATA
		// Show how to get origin names of the table columns from which theses result columns come from.
		// Requires the SQLITE_ENABLE_COLUMN_METADATA preprocessor macro to be
		// also defined at compile times of the SQLite library itself.
		string oname0 = query.getColumnOriginName(0);
		string oname1 = query.getColumnOriginName(1);
		string oname2 = query.getColumnOriginName(2);

		cout << "Origin table 'test' [\"" << oname0 << "\", \"" << oname1 << "\", \"" << oname2 << "\"]\n";
#endif
		// Loop to execute the query step by step, to get one a row of results at a time
		// Demonstrates that inserting column value in a std:ostream is natural
		while (query.executeStep())
			cout << "row (" << query.getColumn(0) << ", \"" << query.getColumn(1) << "\", " << query.getColumn(2) << ")\n";

		// c) Get columns by name
		query.reset();

		// Loop to execute the query step by step, to get one a row of results at a time
		// Demonstrates how to get column value by aliased name (not the original table names, see above)
		while (query.executeStep()) {
			const int id = query.getColumn("test_id");
			const string value = query.getColumn("test_val");
			const double weight = query.getColumn("test_weight");
			cout << "row (" << id << ", \"" << value << ", " << weight << ")\n";
		}

		// d) Uses explicit typed getters instead of auto cast operators
		query.reset();

		// Bind the string value "6" to the first parameter of the SQL query
		query.bind(1, "6");

		// Reuses variables: uses assignement operator in the loop instead of constructor with initialization
		int id = 0;
		string value = "";
		double weight = 0.0;

		while (query.executeStep()) {
			id = query.getColumn(0).getInt();
			value = query.getColumn(1).getText();
			weight = query.getColumn(2).getInt();
			cout << "row (" << id << ", \"" << value << "\", " << weight << ")\n";
		}
  } catch (...) {
		return handleException();
  }

	// Object Oriented Basic example (3/7)
	try {
		// Open the database and compile the query
		Example example;

		// Demonstrates the way to use the same query with different parameter values
		example.listGreaterThan(8);
		example.listGreaterThan(6);
		example.listGreaterThan(2);
	} catch (...) {
		return handleException();
  }

	// The execAndGet wrapper example (4/7)
	try
	{
		SQLite::Database db(dbFileName);

		// WARNING: Be very careful with this dangerous method: you have to
		// make a COPY OF THE result, else it will be destroy before the next line
		// (when the underlying temporary Statement and Column objects are destroyed)
		string value = db.execAndGet("SELECT value FROM test WHERE id=2");
		cout << "execAndGet = " << value << '\n';
	} catch (...) {
		return handleException();
  }

	// Simple batch queries example (5/7)
	try {
		// Open a database file in create/write mode
		SQLite::Database db("test.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
		cout << "SQLite database file '" << db.getFilename() << "' opened successfully\n";

		// Create a new table with an explicit "id" column aliasing the underlying rowid
		db.exec("DROP TABLE IF EXISTS test");
		db.exec("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

		// first row
		int nb = db.exec("INSERT INTO test VALUES (NULL, \"test\")");
		cout << "INSERT INTO test VALUES (NULL, \"test\")\", returned " << nb << '\n';

		// second row
		nb = db.exec("INSERT INTO test VALUES (NULL, \"second\")");
		cout << "INSERT INTO test VALUES (NULL, \"second\")\", returned " << nb << '\n';

		// update the second row
		nb = db.exec("UPDATE test SET value=\"second-updated\" WHERE id='2'");
		cout << "UPDATE test SET value=\"second-updated\" WHERE id='2', returned " << nb << '\n';

		SQLite::Statement query(db, "SELECT * FROM test");
		cout << "SELECT * FROM test\n";

		while (query.executeStep())
			cout << "row (" << query.getColumn(0) << ", \"" << query.getColumn(1) << "\")\n";

		db.exec("DROP TABLE test");
	} catch (...) {
		return handleException();
  }

  remove("test.db3");

	// RAII transaction example (6/7)
	try {
		SQLite::Database db("transaction.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
		db.exec("DROP TABLE IF EXISTS test");

		// Example of a successful transaction
		try {
			SQLite::Transaction tx(db);

			db.exec("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

			int nb = db.exec("INSERT INTO test VALUES (NULL, \"test\")");
			cout << "INSERT INTO test VALUES (NULL, \"test\")\", returned " << nb << '\n';

			tx.commit();
		} catch (...) {
			return handleException();
		}

		// Example of a failed transaction
		try {
			SQLite::Transaction tx(db);

			int nb = db.exec("INSERT INTO test VALUES (NULL, \"second\")");
			cout << "INSERT INTO test VALUES (NULL, \"second\")\", returned " << nb << '\n';

			nb = db.exec("INSERT INTO test ObviousError");
			cout << "INSERT INTO test \"error\", returned " << nb << '\n';

			// Next line will never be executed due to exception raised
			return EXIT_FAILURE;

			tx.commit();
		} catch (...) {
			handleException();
		}

		// Check the results (expect only one row of result, as the second one has been rollbacked by the error)
		SQLite::Statement query(db, "SELECT * FROM test");
		cout << "SELECT * FROM test\n";

		while (query.executeStep())
			cout << "row (" << query.getColumn(0) << ", \"" << query.getColumn(1) << "\")\n";
	} catch (...) {
		return handleException();
  }

	remove("transaction.db3");

	// Binary blob and in-memory database example (7/7)
	try {
		SQLite::Database db(":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		db.exec("DROP TABLE IF EXISTS test");
		db.exec("CREATE TABLE test (id INTEGER PRIMARY KEY, value BLOB)");

		ifstream logo{logoFileName, ios::in | ios::binary};

		if (logo.is_open()) {
			char buffer[16 * 1024];
			logo.read(static_cast<char *>(buffer), sizeof buffer);
			cout << "Blob size = " << logo.gcount() << '\n';

			SQLite::Statement query(db, "INSERT INTO test VALUES (NULL, ?)");
			query.bind(1, &buffer, logo.gcount());

			// Execute the one-step query to insert the blob
			int nb = query.exec();
			cout << "INSERT INTO test VALUES (NULL, ?)\", returned " << nb << '\n';

			logo.close();
		} else {
			cout << "File " << logoFileName << " not found!\n";
			return EXIT_FAILURE;
		}

		ofstream outpng{"out.png", ios::out | ios::binary};

		if (outpng.is_open()) {
			SQLite::Statement query(db, "SELECT * FROM test");
			cout << "SELECT * FROM test\n";

			if (query.executeStep()) {
				SQLite::Column colBlob = query.getColumn(1);
				const void* blob = colBlob.getBlob();
				size_t size = colBlob.getBytes();
				cout << "row (" << query.getColumn(0) << ", size=" << size << ")\n";
				outpng.write(static_cast<char const *>(blob), size);
				outpng.close();
			}
		} else {
			std::cout << "File out.png not created!\n";
			return EXIT_FAILURE;
		}
	} catch (...) {
		return handleException();
  }

	remove("out.png");

#if (__cplusplus >= 201402L) || (defined(_MSC_VER) && (_MSC_VER >= 1900))
	// C++14 and Visual Studio 2015
	// Example with C++14 variadic bind
	try {
		SQLite::Database db(":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		db.exec("DROP TABLE IF EXISTS test");
		db.exec("CREATE TABLE test (id INTEGER PRIMARY KEY, value TEXT)");

		{
			SQLite::Statement query(db, "INSERT INTO test VALUES (?, ?)");

			SQLite::bind(query, 42, "fortytwo");

			int nb = query.exec();
			cout << "INSERT INTO test VALUES (NULL, ?)\", returned " << nb << '\n';
		}

		SQLite::Statement query(db, "SELECT * FROM test");
		cout << "SELECT * FROM test\n";

		if (query.executeStep())
			cout << query.getColumn(0).getInt() << "\t\"" << query.getColumn(1).getText() << "\"\n";
	} catch (...) {
		return handleException();
  }
#endif

  cout << "Done successfully.\n";
  return EXIT_SUCCESS;
}
