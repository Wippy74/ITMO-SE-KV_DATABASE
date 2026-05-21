#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "kernel.h"
#include "dispatcher.h"
#include "parser.h"
#include "result.h"

class StringTest : public ::testing::Test {
protected:
  DataBase db;
  CommandsDispatcher disp;

  OptionalResult Exec(const std::string& line) {
    return disp.Dispatch(db, ParseCommand(line));
  }

  static void ExpectOk(const OptionalResult& r) {
    ASSERT_TRUE(r.has_value());
    ASSERT_TRUE(std::holds_alternative<SimpleString>(*r));
    EXPECT_EQ(std::get<SimpleString>(*r).value, "OK");
  }
  static void ExpectNil(const OptionalResult& r) {
    ASSERT_TRUE(r.has_value());
    EXPECT_TRUE(std::holds_alternative<NilResult>(*r));
  }
  static void ExpectInt(const OptionalResult& r, size_t expected) {
    ASSERT_TRUE(r.has_value());
    ASSERT_TRUE(std::holds_alternative<size_t>(*r));
    EXPECT_EQ(std::get<size_t>(*r), expected);
  }
  static void ExpectString(const OptionalResult& r, const std::string& expected) {
    ASSERT_TRUE(r.has_value());
    ASSERT_TRUE(std::holds_alternative<ResString>(*r));
    EXPECT_EQ(std::get<ResString>(*r).value, expected);
  }
  static void ExpectNullopt(const OptionalResult& r) {
    EXPECT_FALSE(r.has_value());
  }
};

TEST_F(StringTest, SET_ReturnsOk) {
  ExpectOk(Exec("SET key hello"));
}

TEST_F(StringTest, SET_OverwriteSameKey) {
  Exec("SET key first");
  ExpectOk(Exec("SET key second"));
  ExpectString(Exec("GET key"), "second");
}

TEST_F(StringTest, SET_OverwritesWrongTypeKey) {
  Exec("LPUSH key a b c");
  ExpectOk(Exec("SET key string_now"));
  ExpectString(Exec("GET key"), "string_now");
}

TEST_F(StringTest, SET_EmptyValue) {
  ExpectOk(Exec("SET key \"\""));
  ExpectString(Exec("GET key"), "");
}

TEST_F(StringTest, SET_TooFewArgs_NoValue) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SET key"));
  testing::internal::GetCapturedStderr();
}

TEST_F(StringTest, SET_TooFewArgs_NoArgs) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SET"));
  testing::internal::GetCapturedStderr();
}

TEST_F(StringTest, SET_OOM) {
  Exec("CONFIG SET maxmemory 1b");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SET key value"));
  testing::internal::GetCapturedStderr();
}

TEST_F(StringTest, SET_ValueWithSpaces) {
  ExpectOk(Exec("SET key \"hello world\""));
  ExpectString(Exec("GET key"), "hello world");
}

TEST_F(StringTest, GET_ExistingKey) {
  Exec("SET key hello");
  ExpectString(Exec("GET key"), "hello");
}

TEST_F(StringTest, GET_NonExistentKey) {
  ExpectNil(Exec("GET missing"));
}

TEST_F(StringTest, GET_AfterOverwrite) {
  Exec("SET key old");
  Exec("SET key new");
  ExpectString(Exec("GET key"), "new");
}

TEST_F(StringTest, GET_WrongType_List) {
  Exec("LPUSH mylist a");
  ExpectNil(Exec("GET mylist"));
}

TEST_F(StringTest, GET_WrongType_Set) {
  Exec("SADD myset a");
  ExpectNil(Exec("GET myset"));
}

TEST_F(StringTest, GET_TooFewArgs) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GET"));
  testing::internal::GetCapturedStderr();
}

TEST_F(StringTest, GET_EmptyValue) {
  Exec("SET key \"\"");
  ExpectString(Exec("GET key"), "");
}

TEST_F(StringTest, STRLEN_BasicLength) {
  Exec("SET key hello");
  ExpectInt(Exec("STRLEN key"), 5);
}

TEST_F(StringTest, STRLEN_NonExistentKey) {
  ExpectInt(Exec("STRLEN missing"), 0);
}

TEST_F(StringTest, STRLEN_EmptyString) {
  Exec("SET key \"\"");
  ExpectInt(Exec("STRLEN key"), 0);
}

TEST_F(StringTest, STRLEN_WrongType) {
  Exec("LPUSH list a b");
  ExpectInt(Exec("STRLEN list"), 0);
}

TEST_F(StringTest, STRLEN_AfterAppend) {
  Exec("SET key hello");
  Exec("APPEND key world");
  ExpectInt(Exec("STRLEN key"), 10);
}

TEST_F(StringTest, STRLEN_TooFewArgs) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("STRLEN"));
  testing::internal::GetCapturedStderr();
}

TEST_F(StringTest, STRLEN_UnicodeBytes) {
  Exec("SET key ab");
  ExpectInt(Exec("STRLEN key"), 2);
}

TEST_F(StringTest, APPEND_NewKeyCreates) {
  ExpectInt(Exec("APPEND key hello"), 5);
}

TEST_F(StringTest, APPEND_NewKeyGetable) {
  Exec("APPEND key hello");
  ExpectString(Exec("GET key"), "hello");
}

TEST_F(StringTest, APPEND_ToExistingString) {
  Exec("SET key hello");
  ExpectInt(Exec("APPEND key world"), 10);
  ExpectString(Exec("GET key"), "helloworld");
}

TEST_F(StringTest, APPEND_EmptySuffix) {
  Exec("SET key hello");
  ExpectInt(Exec("APPEND key \"\""), 5);
  ExpectString(Exec("GET key"), "hello");
}

TEST_F(StringTest, APPEND_ToEmptyString) {
  Exec("SET key \"\"");
  ExpectInt(Exec("APPEND key hi"), 2);
  ExpectString(Exec("GET key"), "hi");
}

TEST_F(StringTest, APPEND_MultipleAppends) {
  Exec("SET key a");
  Exec("APPEND key b");
  Exec("APPEND key c");
  ExpectString(Exec("GET key"), "abc");
  ExpectInt(Exec("STRLEN key"), 3);
}

TEST_F(StringTest, APPEND_WrongType) {
  Exec("LPUSH list a");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("APPEND list x"));
  testing::internal::GetCapturedStderr();
}

TEST_F(StringTest, APPEND_TooFewArgs_NoValue) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("APPEND key"));
  testing::internal::GetCapturedStderr();
}

TEST_F(StringTest, APPEND_TooFewArgs_NoArgs) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("APPEND"));
  testing::internal::GetCapturedStderr();
}

TEST_F(StringTest, APPEND_OOM) {
  Exec("SET key hello");
  Exec("CONFIG SET maxmemory 1b");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("APPEND key world"));
  testing::internal::GetCapturedStderr();
  ExpectString(Exec("GET key"), "hello");
}

TEST_F(StringTest, EXPIRE_ExistingKeyReturns1) {
  Exec("SET key v");
  ExpectInt(Exec("EXPIRE key 10"), 1);
}

TEST_F(StringTest, EXPIRE_NonExistentKeyReturns0) {
  ExpectInt(Exec("EXPIRE missing 10"), 0);
}

TEST_F(StringTest, EXPIRE_ZeroSecondsError) {
  Exec("SET key v");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("EXPIRE key 0"));
  testing::internal::GetCapturedStderr();
}

TEST_F(StringTest, EXPIRE_NegativeSecondsError) {
  Exec("SET key v");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("EXPIRE key -5"));
  testing::internal::GetCapturedStderr();
}

TEST_F(StringTest, EXPIRE_NonNumericError) {
  Exec("SET key v");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("EXPIRE key abc"));
  testing::internal::GetCapturedStderr();
}

TEST_F(StringTest, EXPIRE_TooFewArgs) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("EXPIRE key"));
  testing::internal::GetCapturedStderr();
}

TEST_F(StringTest, TTL_NoExpiry) {
  Exec("SET key v");
  ExpectInt(Exec("TTL key"), static_cast<size_t>(-1));
}

TEST_F(StringTest, TTL_NonExistentKey) {
  ExpectInt(Exec("TTL missing"), static_cast<size_t>(-2));
}

TEST_F(StringTest, TTL_AfterExpire) {
  Exec("SET key v");
  Exec("EXPIRE key 60");
  auto r = Exec("TTL key");
  ASSERT_TRUE(r.has_value());
  ASSERT_TRUE(std::holds_alternative<size_t>(*r));
  size_t ttl = std::get<size_t>(*r);
  EXPECT_GT(ttl, 0u);
  EXPECT_LE(ttl, 60u);
}

TEST_F(StringTest, TTL_TooFewArgs) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("TTL"));
  testing::internal::GetCapturedStderr();
}

TEST_F(StringTest, TTL_KeyExpiresAndBecomesGone) {
  Exec("SET key v");
  Exec("EXPIRE key 1");
  std::this_thread::sleep_for(std::chrono::milliseconds(1100));
  ExpectNil(Exec("GET key"));
  ExpectInt(Exec("TTL key"), static_cast<size_t>(-2));
}

TEST_F(StringTest, EXPIRE_ResetExpiry) {
  Exec("SET key v");
  Exec("EXPIRE key 1");
  Exec("EXPIRE key 60");
  auto r = Exec("TTL key");
  ASSERT_TRUE(r.has_value());
  size_t ttl = std::get<size_t>(*r);
  EXPECT_GT(ttl, 1u);
}

TEST_F(StringTest, SET_ClearsExistingTTL) {
  Exec("SET key v");
  Exec("EXPIRE key 60");
  Exec("SET key v2");
  ExpectInt(Exec("TTL key"), static_cast<size_t>(-1));
}

TEST_F(StringTest, SET_OverwriteWithExpiry_TTLReset) {
  Exec("SET key v");
  Exec("EXPIRE key 60");
  Exec("SET key new");
  ExpectString(Exec("GET key"), "new");
  ExpectInt(Exec("TTL key"), static_cast<size_t>(-1));
}

TEST_F(StringTest, APPEND_ThenSet_OverwritesResult) {
  Exec("APPEND key hello");
  Exec("APPEND key world");
  Exec("SET key clean");
  ExpectString(Exec("GET key"), "clean");
  ExpectInt(Exec("STRLEN key"), 5);
}

TEST_F(StringTest, GET_WrongType_Geo) {
  Exec("GEOADD key 0 0 a");
  ExpectNil(Exec("GET key"));
}

TEST_F(StringTest, STRLEN_WrongType_Geo) {
  Exec("GEOADD key 0 0 a");
  ExpectInt(Exec("STRLEN key"), 0);
}

TEST_F(StringTest, APPEND_IncreasesStrlen) {
  Exec("SET key abc");
  Exec("APPEND key de");
  ExpectInt(Exec("STRLEN key"), 5);
  ExpectString(Exec("GET key"), "abcde");
}
