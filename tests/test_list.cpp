#include <gtest/gtest.h>
#include "kernel.h"
#include "dispatcher.h"
#include "parser.h"
#include "result.h"

class ListTest : public ::testing::Test {
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
  static void ExpectArray(const OptionalResult& r, const std::vector<std::string>& expected) {
    ASSERT_TRUE(r.has_value());
    ASSERT_TRUE(std::holds_alternative<std::vector<std::string>>(*r));
    EXPECT_EQ(std::get<std::vector<std::string>>(*r), expected);
  }
  static void ExpectNullopt(const OptionalResult& r) {
    EXPECT_FALSE(r.has_value());
  }
};

TEST_F(ListTest, LPUSH_SingleValue_ReturnsSize1) {
  ExpectInt(Exec("LPUSH key a"), 1);
}

TEST_F(ListTest, LPUSH_MultipleValues_ReturnsTotalSize) {
  ExpectInt(Exec("LPUSH key a b c"), 3);
}

TEST_F(ListTest, LPUSH_PushesToFront) {
  Exec("LPUSH key a b c");
  ExpectArray(Exec("LRANGE key 0 -1"), {"c", "b", "a"});
}

TEST_F(ListTest, LPUSH_CreatesListForNewKey) {
  Exec("LPUSH newkey hello");
  ExpectInt(Exec("LLEN newkey"), 1);
}

TEST_F(ListTest, LPUSH_AccumulatesAcrossMultipleCalls) {
  Exec("LPUSH key a");
  ExpectInt(Exec("LPUSH key b"), 2);
  ExpectArray(Exec("LRANGE key 0 -1"), {"b", "a"});
}

TEST_F(ListTest, LPUSH_WrongType_ReturnsNullopt) {
  Exec("SET key str");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LPUSH key val"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LPUSH_TooFewArgs_NoValue) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LPUSH key"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LPUSH_NoArgs) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LPUSH"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LPUSH_OOM) {
  Exec("CONFIG SET maxmemory 1b");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LPUSH key a"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, RPUSH_SingleValue_ReturnsSize1) {
  ExpectInt(Exec("RPUSH key a"), 1);
}

TEST_F(ListTest, RPUSH_MultipleValues_ReturnsTotalSize) {
  ExpectInt(Exec("RPUSH key a b c"), 3);
}

TEST_F(ListTest, RPUSH_PushesToBack) {
  Exec("RPUSH key a b c");
  ExpectArray(Exec("LRANGE key 0 -1"), {"a", "b", "c"});
}

TEST_F(ListTest, RPUSH_CreatesListForNewKey) {
  Exec("RPUSH newkey hello");
  ExpectInt(Exec("LLEN newkey"), 1);
}

TEST_F(ListTest, RPUSH_AccumulatesAcrossMultipleCalls) {
  Exec("RPUSH key a");
  ExpectInt(Exec("RPUSH key b"), 2);
  ExpectArray(Exec("LRANGE key 0 -1"), {"a", "b"});
}

TEST_F(ListTest, RPUSH_WrongType_ReturnsNullopt) {
  Exec("SET key str");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("RPUSH key val"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, RPUSH_TooFewArgs_NoValue) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("RPUSH key"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, RPUSH_NoArgs) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("RPUSH"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LPOP_Single_ReturnsFirstElement) {
  Exec("RPUSH key a b c");
  ExpectString(Exec("LPOP key"), "a");
}

TEST_F(ListTest, LPOP_Single_RemovesElement) {
  Exec("RPUSH key a b c");
  Exec("LPOP key");
  ExpectArray(Exec("LRANGE key 0 -1"), {"b", "c"});
}

TEST_F(ListTest, LPOP_Single_NonExistentKey_ReturnsNil) {
  ExpectNil(Exec("LPOP missing"));
}

TEST_F(ListTest, LPOP_Single_DeletesKeyWhenListBecomesEmpty) {
  Exec("RPUSH key a");
  Exec("LPOP key");
  ExpectInt(Exec("LLEN key"), 0);
}

TEST_F(ListTest, LPOP_Single_WrongType_ReturnsNil) {
  Exec("SET key str");
  ExpectNil(Exec("LPOP key"));
}

TEST_F(ListTest, LPOP_WithCount_ReturnsArray) {
  Exec("RPUSH key a b c");
  ExpectArray(Exec("LPOP key 2"), {"a", "b"});
}

TEST_F(ListTest, LPOP_WithCount_ExceedsSize_ReturnsAll) {
  Exec("RPUSH key a b");
  ExpectArray(Exec("LPOP key 10"), {"a", "b"});
}

TEST_F(ListTest, LPOP_WithCount_Zero_ReturnsEmptyArray) {
  Exec("RPUSH key a");
  ExpectArray(Exec("LPOP key 0"), {});
}

TEST_F(ListTest, LPOP_WithCount_NonExistentKey_ReturnsEmptyArray) {
  ExpectArray(Exec("LPOP missing 3"), {});
}

TEST_F(ListTest, LPOP_WithCount_WrongType_ReturnsEmptyArray) {
  Exec("SET key str");
  ExpectArray(Exec("LPOP key 2"), {});
}

TEST_F(ListTest, LPOP_TooManyArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LPOP key 1 extra"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LPOP_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LPOP"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LPOP_NegativeCount_ReturnsNullopt) {
  Exec("RPUSH key a");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LPOP key -1"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LPOP_NonNumericCount_ReturnsNullopt) {
  Exec("RPUSH key a");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LPOP key abc"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, RPOP_Single_ReturnsLastElement) {
  Exec("RPUSH key a b c");
  ExpectString(Exec("RPOP key"), "c");
}

TEST_F(ListTest, RPOP_Single_RemovesElement) {
  Exec("RPUSH key a b c");
  Exec("RPOP key");
  ExpectArray(Exec("LRANGE key 0 -1"), {"a", "b"});
}

TEST_F(ListTest, RPOP_Single_NonExistentKey_ReturnsNil) {
  ExpectNil(Exec("RPOP missing"));
}

TEST_F(ListTest, RPOP_Single_DeletesKeyWhenListBecomesEmpty) {
  Exec("RPUSH key a");
  Exec("RPOP key");
  ExpectInt(Exec("LLEN key"), 0);
}

TEST_F(ListTest, RPOP_Single_WrongType_ReturnsNil) {
  Exec("SET key str");
  ExpectNil(Exec("RPOP key"));
}

TEST_F(ListTest, RPOP_WithCount_ReturnsArrayFromBack) {
  Exec("RPUSH key a b c");
  ExpectArray(Exec("RPOP key 2"), {"c", "b"});
}

TEST_F(ListTest, RPOP_WithCount_ExceedsSize_ReturnsAll) {
  Exec("RPUSH key a b");
  ExpectArray(Exec("RPOP key 10"), {"b", "a"});
}

TEST_F(ListTest, RPOP_WithCount_Zero_ReturnsEmptyArray) {
  Exec("RPUSH key a");
  ExpectArray(Exec("RPOP key 0"), {});
}

TEST_F(ListTest, RPOP_WithCount_NonExistentKey_ReturnsEmptyArray) {
  ExpectArray(Exec("RPOP missing 3"), {});
}

TEST_F(ListTest, RPOP_TooManyArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("RPOP key 1 extra"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, RPOP_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("RPOP"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, RPOP_NegativeCount_ReturnsNullopt) {
  Exec("RPUSH key a");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("RPOP key -1"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LLEN_NonExistentKey_ReturnsZero) {
  ExpectInt(Exec("LLEN missing"), 0);
}

TEST_F(ListTest, LLEN_AfterPush_ReturnsCorrectSize) {
  Exec("RPUSH key a b c");
  ExpectInt(Exec("LLEN key"), 3);
}

TEST_F(ListTest, LLEN_WrongType_ReturnsZero) {
  Exec("SET key str");
  ExpectInt(Exec("LLEN key"), 0);
}

TEST_F(ListTest, LLEN_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LLEN"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LLEN_AfterPop_DecreasesSize) {
  Exec("RPUSH key a b c");
  Exec("LPOP key");
  ExpectInt(Exec("LLEN key"), 2);
}

TEST_F(ListTest, LRANGE_AllElements) {
  Exec("RPUSH key a b c");
  ExpectArray(Exec("LRANGE key 0 -1"), {"a", "b", "c"});
}

TEST_F(ListTest, LRANGE_PartialRange) {
  Exec("RPUSH key a b c d");
  ExpectArray(Exec("LRANGE key 1 2"), {"b", "c"});
}

TEST_F(ListTest, LRANGE_NegativeStartIndex) {
  Exec("RPUSH key a b c");
  ExpectArray(Exec("LRANGE key -2 -1"), {"b", "c"});
}

TEST_F(ListTest, LRANGE_StartBeyondEnd_ReturnsEmpty) {
  Exec("RPUSH key a b");
  ExpectArray(Exec("LRANGE key 5 10"), {});
}

TEST_F(ListTest, LRANGE_StartGreaterThanStop_ReturnsEmpty) {
  Exec("RPUSH key a b c");
  ExpectArray(Exec("LRANGE key 2 1"), {});
}

TEST_F(ListTest, LRANGE_NonExistentKey_ReturnsEmpty) {
  ExpectArray(Exec("LRANGE missing 0 -1"), {});
}

TEST_F(ListTest, LRANGE_WrongType_ReturnsEmpty) {
  Exec("SET key str");
  ExpectArray(Exec("LRANGE key 0 -1"), {});
}

TEST_F(ListTest, LRANGE_StopBeyondEnd_ClampedToLast) {
  Exec("RPUSH key a b c");
  ExpectArray(Exec("LRANGE key 0 100"), {"a", "b", "c"});
}

TEST_F(ListTest, LRANGE_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LRANGE key 0"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LRANGE_NonNumericIndex_ReturnsNullopt) {
  Exec("RPUSH key a");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LRANGE key abc 2"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LINDEX_ValidPositiveIndex) {
  Exec("RPUSH key a b c");
  ExpectString(Exec("LINDEX key 1"), "b");
}

TEST_F(ListTest, LINDEX_FirstElement) {
  Exec("RPUSH key a b c");
  ExpectString(Exec("LINDEX key 0"), "a");
}

TEST_F(ListTest, LINDEX_ValidNegativeIndex) {
  Exec("RPUSH key a b c");
  ExpectString(Exec("LINDEX key -1"), "c");
}

TEST_F(ListTest, LINDEX_NegativeIndex_Second) {
  Exec("RPUSH key a b c");
  ExpectString(Exec("LINDEX key -2"), "b");
}

TEST_F(ListTest, LINDEX_OutOfBounds_ReturnsNil) {
  Exec("RPUSH key a b");
  ExpectNil(Exec("LINDEX key 5"));
}

TEST_F(ListTest, LINDEX_NegativeOutOfBounds_ReturnsNil) {
  Exec("RPUSH key a b");
  ExpectNil(Exec("LINDEX key -10"));
}

TEST_F(ListTest, LINDEX_NonExistentKey_ReturnsNil) {
  ExpectNil(Exec("LINDEX missing 0"));
}

TEST_F(ListTest, LINDEX_WrongType_ReturnsNil) {
  Exec("SET key str");
  ExpectNil(Exec("LINDEX key 0"));
}

TEST_F(ListTest, LINDEX_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LINDEX key"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LINDEX_NonNumericIndex_ReturnsNullopt) {
  Exec("RPUSH key a");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LINDEX key abc"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LSET_ValidIndex_ReturnsOk) {
  Exec("RPUSH key a b c");
  ExpectOk(Exec("LSET key 1 X"));
}

TEST_F(ListTest, LSET_ValidIndex_UpdatesValue) {
  Exec("RPUSH key a b c");
  Exec("LSET key 1 X");
  ExpectString(Exec("LINDEX key 1"), "X");
}

TEST_F(ListTest, LSET_NegativeIndex_ReturnsOk) {
  Exec("RPUSH key a b c");
  ExpectOk(Exec("LSET key -1 Z"));
}

TEST_F(ListTest, LSET_NegativeIndex_UpdatesValue) {
  Exec("RPUSH key a b c");
  Exec("LSET key -1 Z");
  ExpectString(Exec("LINDEX key 2"), "Z");
}

TEST_F(ListTest, LSET_OutOfBounds_ReturnsNullopt) {
  Exec("RPUSH key a b");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LSET key 5 X"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LSET_NonExistentKey_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LSET missing 0 X"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LSET_WrongType_ReturnsNullopt) {
  Exec("SET key str");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LSET key 0 X"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LSET_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LSET key 0"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LSET_NonNumericIndex_ReturnsNullopt) {
  Exec("RPUSH key a");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LSET key abc X"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LINSERT_Before_InsertsBeforePivot) {
  Exec("RPUSH key a b c");
  ExpectInt(Exec("LINSERT key BEFORE b X"), 4);
  ExpectArray(Exec("LRANGE key 0 -1"), {"a", "X", "b", "c"});
}

TEST_F(ListTest, LINSERT_After_InsertsAfterPivot) {
  Exec("RPUSH key a b c");
  ExpectInt(Exec("LINSERT key AFTER b X"), 4);
  ExpectArray(Exec("LRANGE key 0 -1"), {"a", "b", "X", "c"});
}

TEST_F(ListTest, LINSERT_Before_FirstElement) {
  Exec("RPUSH key a b");
  Exec("LINSERT key BEFORE a X");
  ExpectArray(Exec("LRANGE key 0 -1"), {"X", "a", "b"});
}

TEST_F(ListTest, LINSERT_After_LastElement) {
  Exec("RPUSH key a b");
  Exec("LINSERT key AFTER b X");
  ExpectArray(Exec("LRANGE key 0 -1"), {"a", "b", "X"});
}

TEST_F(ListTest, LINSERT_PivotNotFound_ReturnsMinus1) {
  Exec("RPUSH key a b c");
  ExpectInt(Exec("LINSERT key BEFORE z X"), static_cast<size_t>(-1));
}

TEST_F(ListTest, LINSERT_KeyNotFound_ReturnsZero) {
  ExpectInt(Exec("LINSERT missing BEFORE a X"), 0);
}

TEST_F(ListTest, LINSERT_CaseInsensitiveDirection) {
  Exec("RPUSH key a b");
  ExpectInt(Exec("LINSERT key before b X"), 3);
  ExpectArray(Exec("LRANGE key 0 -1"), {"a", "X", "b"});
}

TEST_F(ListTest, LINSERT_InvalidDirection_ReturnsNullopt) {
  Exec("RPUSH key a b");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LINSERT key MIDDLE b X"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LINSERT_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LINSERT key BEFORE a"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LINSERT_OOM_ReturnsNullopt) {
  Exec("RPUSH key a b");
  Exec("CONFIG SET maxmemory 1b");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("LINSERT key BEFORE a X"));
  testing::internal::GetCapturedStderr();
}

TEST_F(ListTest, LINSERT_DuplicatePivot_UsesFirstOccurrence) {
  Exec("RPUSH key a b b c");
  Exec("LINSERT key BEFORE b X");
  ExpectArray(Exec("LRANGE key 0 -1"), {"a", "X", "b", "b", "c"});
}

TEST_F(ListTest, LPUSH_RPUSH_Interleaved_CorrectOrder) {
  Exec("RPUSH key a");
  Exec("LPUSH key b");
  Exec("RPUSH key c");
  Exec("LPUSH key d");
  ExpectArray(Exec("LRANGE key 0 -1"), {"d", "b", "a", "c"});
}

TEST_F(ListTest, LRANGE_AfterLSet_ReflectsChange) {
  Exec("RPUSH key a b c");
  Exec("LSET key 1 Z");
  ExpectArray(Exec("LRANGE key 0 -1"), {"a", "Z", "c"});
}

TEST_F(ListTest, LSET_ShrinkingValue_SucceedsAtMemoryLimit) {
  Exec("RPUSH key longvalue");
  Exec("CONFIG SET maxmemory 1b");
  ExpectOk(Exec("LSET key 0 x"));
}

TEST_F(ListTest, EXPIRE_OnListKey_Returns1) {
  Exec("RPUSH key a b c");
  ExpectInt(Exec("EXPIRE key 60"), 1);
}

TEST_F(ListTest, TTL_OnListKey_AfterExpire_ReturnsPositive) {
  Exec("RPUSH key a b c");
  Exec("EXPIRE key 60");
  auto r = Exec("TTL key");
  ASSERT_TRUE(r.has_value());
  ASSERT_TRUE(std::holds_alternative<size_t>(*r));
  size_t ttl = std::get<size_t>(*r);
  EXPECT_GT(ttl, 0u);
  EXPECT_LE(ttl, 60u);
}

TEST_F(ListTest, TTL_OnListKey_NoExpiry_ReturnsMinus1) {
  Exec("RPUSH key a");
  ExpectInt(Exec("TTL key"), static_cast<size_t>(-1));
}
