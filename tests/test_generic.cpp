#include <gtest/gtest.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include "kernel.h"
#include "dispatcher.h"
#include "parser.h"
#include "result.h"

class GenericTest : public ::testing::Test {
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
  static void ExpectStatus(const OptionalResult& r, const std::string& expected) {
    ASSERT_TRUE(r.has_value());
    ASSERT_TRUE(std::holds_alternative<SimpleString>(*r));
    EXPECT_EQ(std::get<SimpleString>(*r).value, expected);
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
  static void ExpectArray(const OptionalResult& r, const std::vector<std::string>& expected) {
    ASSERT_TRUE(r.has_value());
    ASSERT_TRUE(std::holds_alternative<std::vector<std::string>>(*r));
    EXPECT_EQ(std::get<std::vector<std::string>>(*r), expected);
  }
  static void ExpectNullopt(const OptionalResult& r) {
    EXPECT_FALSE(r.has_value());
  }
};

TEST_F(GenericTest, DEL_ExistingKey_Returns1) {
  Exec("SET key val");
  ExpectInt(Exec("DEL key"), 1);
}

TEST_F(GenericTest, DEL_NonExistentKey_Returns0) {
  ExpectInt(Exec("DEL missing"), 0);
}

TEST_F(GenericTest, DEL_MultipleKeys_ReturnsDeletedCount) {
  Exec("SET a 1");
  Exec("SET b 2");
  ExpectInt(Exec("DEL a b missing"), 2);
}

TEST_F(GenericTest, DEL_RemovesKey) {
  Exec("SET key val");
  Exec("DEL key");
  ExpectInt(Exec("EXISTS key"), 0);
}

TEST_F(GenericTest, DEL_SameKeyTwice_CountsOnce) {
  Exec("SET key val");
  ExpectInt(Exec("DEL key key"), 1);
}

TEST_F(GenericTest, DEL_WorksOnList) {
  Exec("RPUSH list a b c");
  ExpectInt(Exec("DEL list"), 1);
  ExpectInt(Exec("EXISTS list"), 0);
}

TEST_F(GenericTest, DEL_WorksOnSet) {
  Exec("SADD myset a b");
  ExpectInt(Exec("DEL myset"), 1);
}

TEST_F(GenericTest, DEL_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("DEL"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GenericTest, EXISTS_ExistingKey_Returns1) {
  Exec("SET key val");
  ExpectInt(Exec("EXISTS key"), 1);
}

TEST_F(GenericTest, EXISTS_NonExistentKey_Returns0) {
  ExpectInt(Exec("EXISTS missing"), 0);
}

TEST_F(GenericTest, EXISTS_MultipleKeys_ReturnsPresentCount) {
  Exec("SET a 1");
  Exec("SET b 2");
  ExpectInt(Exec("EXISTS a b missing"), 2);
}

TEST_F(GenericTest, EXISTS_SameKeyTwice_CountsBoth) {
  Exec("SET key val");
  ExpectInt(Exec("EXISTS key key"), 2);
}

TEST_F(GenericTest, EXISTS_AfterDel_Returns0) {
  Exec("SET key val");
  Exec("DEL key");
  ExpectInt(Exec("EXISTS key"), 0);
}

TEST_F(GenericTest, EXISTS_WorksOnList) {
  Exec("RPUSH list a");
  ExpectInt(Exec("EXISTS list"), 1);
}

TEST_F(GenericTest, EXISTS_WorksOnSet) {
  Exec("SADD myset a");
  ExpectInt(Exec("EXISTS myset"), 1);
}

TEST_F(GenericTest, EXISTS_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("EXISTS"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GenericTest, DBSIZE_EmptyDb_Returns0) {
  ExpectInt(Exec("DBSIZE"), 0);
}

TEST_F(GenericTest, DBSIZE_AfterSet_Returns1) {
  Exec("SET key val");
  ExpectInt(Exec("DBSIZE"), 1);
}

TEST_F(GenericTest, DBSIZE_MultipleKeys_ReturnsCorrectCount) {
  Exec("SET a 1");
  Exec("SET b 2");
  Exec("SET c 3");
  ExpectInt(Exec("DBSIZE"), 3);
}

TEST_F(GenericTest, DBSIZE_AfterDel_Decreases) {
  Exec("SET a 1");
  Exec("SET b 2");
  Exec("DEL a");
  ExpectInt(Exec("DBSIZE"), 1);
}

TEST_F(GenericTest, DBSIZE_AfterFlushDB_Returns0) {
  Exec("SET a 1");
  Exec("SET b 2");
  Exec("FLUSHDB");
  ExpectInt(Exec("DBSIZE"), 0);
}

TEST_F(GenericTest, DBSIZE_MixedTypes) {
  Exec("SET str val");
  Exec("RPUSH list a");
  Exec("SADD myset x");
  ExpectInt(Exec("DBSIZE"), 3);
}

TEST_F(GenericTest, FLUSHDB_ReturnsOk) {
  ExpectOk(Exec("FLUSHDB"));
}

TEST_F(GenericTest, FLUSHDB_ClearsAllKeys) {
  Exec("SET a 1");
  Exec("RPUSH b x");
  Exec("SADD c y");
  Exec("FLUSHDB");
  ExpectInt(Exec("DBSIZE"), 0);
}

TEST_F(GenericTest, FLUSHDB_EmptyDb_ReturnsOk) {
  ExpectOk(Exec("FLUSHDB"));
}

TEST_F(GenericTest, TYPE_StringKey_ReturnsString) {
  Exec("SET key val");
  ExpectStatus(Exec("TYPE key"), "string");
}

TEST_F(GenericTest, TYPE_ListKey_ReturnsList) {
  Exec("RPUSH key a");
  ExpectStatus(Exec("TYPE key"), "list");
}

TEST_F(GenericTest, TYPE_SetKey_ReturnsSet) {
  Exec("SADD key a");
  ExpectStatus(Exec("TYPE key"), "set");
}

TEST_F(GenericTest, TYPE_GeoKey_ReturnsGeo) {
  Exec("GEOADD key 13.361 38.115 palermo");
  ExpectStatus(Exec("TYPE key"), "geospacial index");
}

TEST_F(GenericTest, TYPE_NonExistentKey_ReturnsNone) {
  ExpectStatus(Exec("TYPE missing"), "none");
}

TEST_F(GenericTest, TYPE_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("TYPE"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GenericTest, KEYS_StarPattern_ReturnsAllSorted) {
  Exec("SET apple 1");
  Exec("SET banana 2");
  Exec("SET cherry 3");
  ExpectArray(Exec("KEYS *"), {"apple", "banana", "cherry"});
}

TEST_F(GenericTest, KEYS_ExactMatch) {
  Exec("SET key val");
  Exec("SET other val");
  ExpectArray(Exec("KEYS key"), {"key"});
}

TEST_F(GenericTest, KEYS_QuestionMarkWildcard) {
  Exec("SET h1llo val");
  Exec("SET h2llo val");
  Exec("SET hello val");
  ExpectArray(Exec("KEYS h?llo"), {"h1llo", "h2llo", "hello"});
}

TEST_F(GenericTest, KEYS_PrefixPattern) {
  Exec("SET user:1 a");
  Exec("SET user:2 b");
  Exec("SET post:1 c");
  ExpectArray(Exec("KEYS user:*"), {"user:1", "user:2"});
}

TEST_F(GenericTest, KEYS_SuffixPattern) {
  Exec("SET foo:1 a");
  Exec("SET bar:1 b");
  Exec("SET foo:2 c");
  ExpectArray(Exec("KEYS *:1"), {"bar:1", "foo:1"});
}

TEST_F(GenericTest, KEYS_NoMatch_ReturnsEmpty) {
  Exec("SET key val");
  ExpectArray(Exec("KEYS zzz"), {});
}

TEST_F(GenericTest, KEYS_EmptyDb_ReturnsEmpty) {
  ExpectArray(Exec("KEYS *"), {});
}

TEST_F(GenericTest, KEYS_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("KEYS"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GenericTest, MEMORY_USAGE_NonExistentKey_ReturnsNil) {
  ExpectNil(Exec("MEMORY USAGE missing"));
}

TEST_F(GenericTest, MEMORY_USAGE_StringKey_ReturnsPositiveSize) {
  Exec("SET key hello");
  auto r = Exec("MEMORY USAGE key");
  ASSERT_TRUE(r.has_value());
  ASSERT_TRUE(std::holds_alternative<size_t>(*r));
  EXPECT_GT(std::get<size_t>(*r), 0u);
}

TEST_F(GenericTest, MEMORY_USAGE_ListKey_ReturnsPositiveSize) {
  Exec("RPUSH list a b c");
  auto r = Exec("MEMORY USAGE list");
  ASSERT_TRUE(r.has_value());
  ASSERT_TRUE(std::holds_alternative<size_t>(*r));
  EXPECT_GT(std::get<size_t>(*r), 0u);
}

TEST_F(GenericTest, MEMORY_USAGE_UnknownSubcommand_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("MEMORY STATS"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GenericTest, MEMORY_USAGE_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("MEMORY"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GenericTest, CONFIG_SET_Bytes_ReturnsOk) {
  ExpectOk(Exec("CONFIG SET maxmemory 1024"));
}

TEST_F(GenericTest, CONFIG_SET_WithKbSuffix_ReturnsOk) {
  ExpectOk(Exec("CONFIG SET maxmemory 1kb"));
}

TEST_F(GenericTest, CONFIG_SET_WithMbSuffix_ReturnsOk) {
  ExpectOk(Exec("CONFIG SET maxmemory 1mb"));
}

TEST_F(GenericTest, CONFIG_GET_ReturnsCurrentValue) {
  Exec("CONFIG SET maxmemory 1024");
  ExpectArray(Exec("CONFIG GET maxmemory"), {"maxmemory", "1024"});
}

TEST_F(GenericTest, CONFIG_GET_AfterKbSet_Returns1024) {
  Exec("CONFIG SET maxmemory 1kb");
  ExpectArray(Exec("CONFIG GET maxmemory"), {"maxmemory", "1024"});
}

TEST_F(GenericTest, CONFIG_GET_AfterMbSet_Returns1048576) {
  Exec("CONFIG SET maxmemory 1mb");
  ExpectArray(Exec("CONFIG GET maxmemory"), {"maxmemory", "1048576"});
}

TEST_F(GenericTest, CONFIG_SET_InvalidValue_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("CONFIG SET maxmemory notanumber"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GenericTest, CONFIG_SET_NegativeValue_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("CONFIG SET maxmemory -1"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GenericTest, CONFIG_GET_UnsupportedParam_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("CONFIG GET unknownparam"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GenericTest, CONFIG_SET_UnsupportedParam_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("CONFIG SET unknownparam 123"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GenericTest, CONFIG_UnknownSubcommand_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("CONFIG RESET maxmemory"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GenericTest, CONFIG_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("CONFIG SET"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GenericTest, CONFIG_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("CONFIG"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GenericTest, CONFIG_CaseInsensitiveSubcommand) {
  ExpectOk(Exec("config set maxmemory 512"));
  ExpectArray(Exec("config get maxmemory"), {"maxmemory", "512"});
}

TEST_F(GenericTest, EXPIRE_OnListKey_Returns1) {
  Exec("RPUSH list a b c");
  ExpectInt(Exec("EXPIRE list 60"), 1);
}

TEST_F(GenericTest, EXPIRE_OnSetKey_Returns1) {
  Exec("SADD myset a b");
  ExpectInt(Exec("EXPIRE myset 60"), 1);
}

TEST_F(GenericTest, EXPIRE_OnGeoKey_Returns1) {
  Exec("GEOADD geo 0 0 a");
  ExpectInt(Exec("EXPIRE geo 60"), 1);
}

TEST_F(GenericTest, TTL_OnListKey_AfterExpire_ReturnsPositive) {
  Exec("RPUSH list a b");
  Exec("EXPIRE list 60");
  auto r = Exec("TTL list");
  ASSERT_TRUE(r.has_value());
  ASSERT_TRUE(std::holds_alternative<size_t>(*r));
  EXPECT_GT(std::get<size_t>(*r), 0u);
}

TEST_F(GenericTest, TTL_OnSetKey_NoExpiry_ReturnsMinus1) {
  Exec("SADD myset a");
  ExpectInt(Exec("TTL myset"), static_cast<size_t>(-1));
}

TEST_F(GenericTest, TTL_OnGeoKey_NoExpiry_ReturnsMinus1) {
  Exec("GEOADD geo 0 0 a");
  ExpectInt(Exec("TTL geo"), static_cast<size_t>(-1));
}

TEST_F(GenericTest, KEYS_DoesNotReturnExpiredKey) {
  Exec("SET key v");
  Exec("EXPIRE key 1");
  std::this_thread::sleep_for(std::chrono::milliseconds(1100));
  ExpectArray(Exec("KEYS *"), {});
}

TEST_F(GenericTest, CONFIG_SET_Zero_RemovesMemoryLimit) {
  Exec("CONFIG SET maxmemory 1b");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SET key value"));
  testing::internal::GetCapturedStderr();
  Exec("CONFIG SET maxmemory 0");
  ExpectOk(Exec("SET key value"));
}

TEST_F(GenericTest, DEL_GeoKey_Returns1) {
  Exec("GEOADD geo 0 0 a");
  ExpectInt(Exec("DEL geo"), 1);
  ExpectInt(Exec("EXISTS geo"), 0);
}
