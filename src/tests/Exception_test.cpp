#include <string>
#include <gtest/gtest.h>
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Exception.h>

TEST(Exception, copy) {
  const SQLite::Exception ex1("some error");
  const SQLite::Exception ex2 = ex1;

  EXPECT_STREQ(ex1.what(), ex2.what());
  EXPECT_EQ(ex1.code(), ex2.code());
  EXPECT_EQ(ex1.extendedCode(), ex2.extendedCode());
}

// See http://eel.is/c++draft/exception#2 or http://www.cplusplus.com/reference/exception/exception/operator=/
// an assignment operator is expected to be available.
TEST(Exception, assignment) {
  const SQLite::Exception ex1("some error");
  SQLite::Exception ex2("some error 2");

  ex2 = ex1;

  EXPECT_STREQ(ex1.what(), ex2.what());
  EXPECT_STREQ("some error", ex2.what());
  EXPECT_EQ(ex1.code(), ex2.code());
  EXPECT_EQ(ex1.extendedCode(), ex2.extendedCode());
}

TEST(Exception, throw_catch) {
  const char message[] = "some error";

  try {
    throw SQLite::Exception(message);
  } catch (std::runtime_error const &ex) {
    EXPECT_STREQ(ex.what(), message);
  }
}
