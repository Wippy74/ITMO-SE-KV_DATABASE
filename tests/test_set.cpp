#include <gtest/gtest.h>
#include <algorithm>
#include "kernel.h"
#include "dispatcher.h"
#include "parser.h"
#include "result.h"

class SetTest : public ::testing::Test {
protected:
  DataBase db;
  CommandsDispatcher disp;

  OptionalResult Exec(const std::string& line) {
    return disp.Dispatch(db, ParseCommand(line));
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
  static void ExpectArray(const OptionalResult& r, std::vector<std::string> expected) {
    ASSERT_TRUE(r.has_value());
    ASSERT_TRUE(std::holds_alternative<std::vector<std::string>>(*r));
    auto got = std::get<std::vector<std::string>>(*r);
    std::sort(got.begin(), got.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(got, expected);
  }
  static void ExpectNullopt(const OptionalResult& r) {
    EXPECT_FALSE(r.has_value());
  }
};

TEST_F(SetTest, SADD_SingleMember_Returns1) {
  ExpectInt(Exec("SADD key a"), 1);
}

TEST_F(SetTest, SADD_MultipleNewMembers_ReturnsCount) {
  ExpectInt(Exec("SADD key a b c"), 3);
}

TEST_F(SetTest, SADD_DuplicateMember_Returns0) {
  Exec("SADD key a");
  ExpectInt(Exec("SADD key a"), 0);
}

TEST_F(SetTest, SADD_MixedNewAndDuplicate_ReturnsNewOnly) {
  Exec("SADD key a b");
  ExpectInt(Exec("SADD key b c"), 1);
}

TEST_F(SetTest, SADD_CreatesKeyForNewSet) {
  Exec("SADD key a");
  ExpectInt(Exec("SCARD key"), 1);
}

TEST_F(SetTest, SADD_WrongType_ReturnsNullopt) {
  Exec("SET key str");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SADD key a"));
  testing::internal::GetCapturedStderr();
}

TEST_F(SetTest, SADD_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SADD key"));
  testing::internal::GetCapturedStderr();
}

TEST_F(SetTest, SADD_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SADD"));
  testing::internal::GetCapturedStderr();
}

TEST_F(SetTest, SADD_OOM_ReturnsNullopt) {
  Exec("CONFIG SET maxmemory 1b");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SADD key a"));
  testing::internal::GetCapturedStderr();
}

TEST_F(SetTest, SREM_ExistingMember_Returns1) {
  Exec("SADD key a b");
  ExpectInt(Exec("SREM key a"), 1);
}

TEST_F(SetTest, SREM_NonExistentMember_Returns0) {
  Exec("SADD key a");
  ExpectInt(Exec("SREM key z"), 0);
}

TEST_F(SetTest, SREM_MultipleMembers_ReturnsRemovedCount) {
  Exec("SADD key a b c");
  ExpectInt(Exec("SREM key a b z"), 2);
}

TEST_F(SetTest, SREM_RemovesFromSet) {
  Exec("SADD key a b c");
  Exec("SREM key b");
  ExpectArray(Exec("SMEMBERS key"), {"a", "c"});
}

TEST_F(SetTest, SREM_NonExistentKey_Returns0) {
  ExpectInt(Exec("SREM missing a"), 0);
}

TEST_F(SetTest, SREM_WrongType_Returns0) {
  Exec("SET key str");
  ExpectInt(Exec("SREM key a"), 0);
}

TEST_F(SetTest, SREM_RemoveAll_DeletesKey) {
  Exec("SADD key a");
  Exec("SREM key a");
  ExpectInt(Exec("SCARD key"), 0);
}

TEST_F(SetTest, SREM_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SREM key"));
  testing::internal::GetCapturedStderr();
}

TEST_F(SetTest, SCARD_NonExistentKey_Returns0) {
  ExpectInt(Exec("SCARD missing"), 0);
}

TEST_F(SetTest, SCARD_AfterAdd_ReturnsCorrectSize) {
  Exec("SADD key a b c");
  ExpectInt(Exec("SCARD key"), 3);
}

TEST_F(SetTest, SCARD_WrongType_Returns0) {
  Exec("SET key str");
  ExpectInt(Exec("SCARD key"), 0);
}

TEST_F(SetTest, SCARD_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SCARD"));
  testing::internal::GetCapturedStderr();
}

TEST_F(SetTest, SCARD_AfterRemove_DecreasesSize) {
  Exec("SADD key a b c");
  Exec("SREM key a");
  ExpectInt(Exec("SCARD key"), 2);
}

TEST_F(SetTest, SISMEMBER_MemberExists_Returns1) {
  Exec("SADD key a b c");
  ExpectInt(Exec("SISMEMBER key b"), 1);
}

TEST_F(SetTest, SISMEMBER_MemberAbsent_Returns0) {
  Exec("SADD key a b");
  ExpectInt(Exec("SISMEMBER key z"), 0);
}

TEST_F(SetTest, SISMEMBER_NonExistentKey_Returns0) {
  ExpectInt(Exec("SISMEMBER missing a"), 0);
}

TEST_F(SetTest, SISMEMBER_WrongType_Returns0) {
  Exec("SET key str");
  ExpectInt(Exec("SISMEMBER key a"), 0);
}

TEST_F(SetTest, SISMEMBER_AfterRemove_Returns0) {
  Exec("SADD key a");
  Exec("SREM key a");
  ExpectInt(Exec("SISMEMBER key a"), 0);
}

TEST_F(SetTest, SISMEMBER_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SISMEMBER key"));
  testing::internal::GetCapturedStderr();
}

TEST_F(SetTest, SMEMBERS_ReturnsAllMembers) {
  Exec("SADD key a b c");
  ExpectArray(Exec("SMEMBERS key"), {"a", "b", "c"});
}

TEST_F(SetTest, SMEMBERS_NonExistentKey_ReturnsEmpty) {
  ExpectArray(Exec("SMEMBERS missing"), {});
}

TEST_F(SetTest, SMEMBERS_WrongType_ReturnsEmpty) {
  Exec("SET key str");
  ExpectArray(Exec("SMEMBERS key"), {});
}

TEST_F(SetTest, SMEMBERS_SingleMember) {
  Exec("SADD key only");
  ExpectArray(Exec("SMEMBERS key"), {"only"});
}

TEST_F(SetTest, SMEMBERS_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SMEMBERS"));
  testing::internal::GetCapturedStderr();
}

TEST_F(SetTest, SMEMBERS_AfterRemove_ReflectsChange) {
  Exec("SADD key a b c");
  Exec("SREM key b");
  ExpectArray(Exec("SMEMBERS key"), {"a", "c"});
}

TEST_F(SetTest, SMOVE_MovesExistingMember_Returns1) {
  Exec("SADD src a b");
  Exec("SADD dst x");
  ExpectInt(Exec("SMOVE src dst a"), 1);
}

TEST_F(SetTest, SMOVE_MemberRemovedFromSrc) {
  Exec("SADD src a b");
  Exec("SMOVE src dst a");
  ExpectInt(Exec("SISMEMBER src a"), 0);
}

TEST_F(SetTest, SMOVE_MemberAddedToDst) {
  Exec("SADD src a b");
  Exec("SADD dst x");
  Exec("SMOVE src dst a");
  ExpectInt(Exec("SISMEMBER dst a"), 1);
}

TEST_F(SetTest, SMOVE_SrcDoesNotExist_Returns0) {
  Exec("SADD dst x");
  ExpectInt(Exec("SMOVE missing dst a"), 0);
}

TEST_F(SetTest, SMOVE_MemberNotInSrc_Returns0) {
  Exec("SADD src a");
  Exec("SADD dst x");
  ExpectInt(Exec("SMOVE src dst z"), 0);
}

TEST_F(SetTest, SMOVE_CreatesDstIfNotExists) {
  Exec("SADD src a b");
  Exec("SMOVE src dst a");
  ExpectInt(Exec("SISMEMBER dst a"), 1);
}

TEST_F(SetTest, SMOVE_SrcBecomesEmpty_KeyDeleted) {
  Exec("SADD src a");
  Exec("SMOVE src dst a");
  ExpectInt(Exec("SCARD src"), 0);
}

TEST_F(SetTest, SMOVE_DstWrongType_ReturnsNullopt) {
  Exec("SADD src a");
  Exec("SET dst str");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SMOVE src dst a"));
  testing::internal::GetCapturedStderr();
}

TEST_F(SetTest, SMOVE_MemberAlreadyInDst_StillReturns1) {
  Exec("SADD src a b");
  Exec("SADD dst a");
  ExpectInt(Exec("SMOVE src dst a"), 1);
  ExpectInt(Exec("SCARD dst"), 1);
}

TEST_F(SetTest, SMOVE_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SMOVE src dst"));
  testing::internal::GetCapturedStderr();
}

TEST_F(SetTest, SDIFF_BasicDifference) {
  Exec("SADD a 1 2 3");
  Exec("SADD b 2 3 4");
  ExpectArray(Exec("SDIFF a b"), {"1"});
}

TEST_F(SetTest, SDIFF_NoOverlap_ReturnsAll) {
  Exec("SADD a 1 2");
  Exec("SADD b 3 4");
  ExpectArray(Exec("SDIFF a b"), {"1", "2"});
}

TEST_F(SetTest, SDIFF_SubtractNonExistentKey_ReturnsAll) {
  Exec("SADD a 1 2 3");
  ExpectArray(Exec("SDIFF a missing"), {"1", "2", "3"});
}

TEST_F(SetTest, SDIFF_FirstKeyNotExist_ReturnsEmpty) {
  Exec("SADD b 1 2");
  ExpectArray(Exec("SDIFF missing b"), {});
}

TEST_F(SetTest, SDIFF_MultipleSubtracted) {
  Exec("SADD a 1 2 3 4");
  Exec("SADD b 2");
  Exec("SADD c 3");
  ExpectArray(Exec("SDIFF a b c"), {"1", "4"});
}

TEST_F(SetTest, SDIFF_WrongTypeFirstKey_ReturnsEmpty) {
  Exec("SET a str");
  Exec("SADD b 1");
  ExpectArray(Exec("SDIFF a b"), {});
}

TEST_F(SetTest, SDIFF_WrongTypeOtherKey_Ignored) {
  Exec("SADD a 1 2 3");
  Exec("SET b str");
  ExpectArray(Exec("SDIFF a b"), {"1", "2", "3"});
}

TEST_F(SetTest, SDIFF_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SDIFF"));
  testing::internal::GetCapturedStderr();
}

TEST_F(SetTest, SINTER_BasicIntersection) {
  Exec("SADD a 1 2 3");
  Exec("SADD b 2 3 4");
  ExpectArray(Exec("SINTER a b"), {"2", "3"});
}

TEST_F(SetTest, SINTER_NoCommonElements_ReturnsEmpty) {
  Exec("SADD a 1 2");
  Exec("SADD b 3 4");
  ExpectArray(Exec("SINTER a b"), {});
}

TEST_F(SetTest, SINTER_FirstKeyNotExist_ReturnsEmpty) {
  Exec("SADD b 1 2");
  ExpectArray(Exec("SINTER missing b"), {});
}

TEST_F(SetTest, SINTER_OtherKeyNotExist_ReturnsEmpty) {
  Exec("SADD a 1 2");
  ExpectArray(Exec("SINTER a missing"), {});
}

TEST_F(SetTest, SINTER_MultipleKeys) {
  Exec("SADD a 1 2 3 4");
  Exec("SADD b 2 3 4 5");
  Exec("SADD c 3 4 5 6");
  ExpectArray(Exec("SINTER a b c"), {"3", "4"});
}

TEST_F(SetTest, SINTER_WrongTypeFirstKey_ReturnsEmpty) {
  Exec("SET a str");
  Exec("SADD b 1");
  ExpectArray(Exec("SINTER a b"), {});
}

TEST_F(SetTest, SINTER_WrongTypeOtherKey_ReturnsEmpty) {
  Exec("SADD a 1 2");
  Exec("SET b str");
  ExpectArray(Exec("SINTER a b"), {});
}

TEST_F(SetTest, SINTER_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SINTER"));
  testing::internal::GetCapturedStderr();
}

TEST_F(SetTest, SUNION_BasicUnion) {
  Exec("SADD a 1 2");
  Exec("SADD b 3 4");
  ExpectArray(Exec("SUNION a b"), {"1", "2", "3", "4"});
}

TEST_F(SetTest, SUNION_DeduplicatesCommonMembers) {
  Exec("SADD a 1 2 3");
  Exec("SADD b 2 3 4");
  ExpectArray(Exec("SUNION a b"), {"1", "2", "3", "4"});
}

TEST_F(SetTest, SUNION_NonExistentKeyIgnored) {
  Exec("SADD a 1 2");
  ExpectArray(Exec("SUNION a missing"), {"1", "2"});
}

TEST_F(SetTest, SUNION_WrongTypeKeyIgnored) {
  Exec("SADD a 1 2");
  Exec("SET b str");
  ExpectArray(Exec("SUNION a b"), {"1", "2"});
}

TEST_F(SetTest, SUNION_SingleKey_SameAsSmembers) {
  Exec("SADD a 1 2 3");
  ExpectArray(Exec("SUNION a"), {"1", "2", "3"});
}

TEST_F(SetTest, SUNION_MultipleKeys) {
  Exec("SADD a 1");
  Exec("SADD b 2");
  Exec("SADD c 3");
  ExpectArray(Exec("SUNION a b c"), {"1", "2", "3"});
}

TEST_F(SetTest, SUNION_AllNonExistent_ReturnsEmpty) {
  ExpectArray(Exec("SUNION missing1 missing2"), {});
}

TEST_F(SetTest, SUNION_NoArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("SUNION"));
  testing::internal::GetCapturedStderr();
}

TEST_F(SetTest, SDIFF_SingleKey_ReturnsAllMembers) {
  Exec("SADD a 1 2 3");
  ExpectArray(Exec("SDIFF a"), {"1", "2", "3"});
}

TEST_F(SetTest, SINTER_SingleKey_ReturnsAllMembers) {
  Exec("SADD a 1 2 3");
  ExpectArray(Exec("SINTER a"), {"1", "2", "3"});
}

TEST_F(SetTest, SUNION_SingleKey_ReturnsAllMembers) {
  Exec("SADD a 1 2 3");
  ExpectArray(Exec("SUNION a"), {"1", "2", "3"});
}

TEST_F(SetTest, SADD_DuplicatesDoNotIncreaseSCARD) {
  Exec("SADD key a a a");
  ExpectInt(Exec("SCARD key"), 1);
}

TEST_F(SetTest, EXPIRE_OnSetKey_Returns1) {
  Exec("SADD key a b c");
  ExpectInt(Exec("EXPIRE key 60"), 1);
}

TEST_F(SetTest, TTL_OnSetKey_AfterExpire_ReturnsPositive) {
  Exec("SADD key a b c");
  Exec("EXPIRE key 60");
  auto r = Exec("TTL key");
  ASSERT_TRUE(r.has_value());
  ASSERT_TRUE(std::holds_alternative<size_t>(*r));
  size_t ttl = std::get<size_t>(*r);
  EXPECT_GT(ttl, 0u);
  EXPECT_LE(ttl, 60u);
}

TEST_F(SetTest, TTL_OnSetKey_NoExpiry_ReturnsMinus1) {
  Exec("SADD key a");
  ExpectInt(Exec("TTL key"), static_cast<size_t>(-1));
}

TEST_F(SetTest, SREM_ThenSISMEMBER_Returns0) {
  Exec("SADD key a b c");
  Exec("SREM key b");
  ExpectInt(Exec("SISMEMBER key b"), 0);
  ExpectInt(Exec("SISMEMBER key a"), 1);
}
