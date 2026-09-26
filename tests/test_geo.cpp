#include <gtest/gtest.h>
#include <cmath>
#include "kernel.h"
#include "dispatcher.h"
#include "parser.h"
#include "result.h"

class GeoTest : public ::testing::Test {
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
  static void ExpectStatus(const OptionalResult& r, const std::string& expected) {
    ASSERT_TRUE(r.has_value());
    ASSERT_TRUE(std::holds_alternative<SimpleString>(*r));
    EXPECT_EQ(std::get<SimpleString>(*r).value, expected);
  }
  static void ExpectStatusContains(const OptionalResult& r, const std::string& substr) {
    ASSERT_TRUE(r.has_value());
    ASSERT_TRUE(std::holds_alternative<SimpleString>(*r));
    EXPECT_NE(std::get<SimpleString>(*r).value.find(substr), std::string::npos);
  }
  static void ExpectArray(const OptionalResult& r, const std::vector<std::string>& expected) {
    ASSERT_TRUE(r.has_value());
    ASSERT_TRUE(std::holds_alternative<std::vector<std::string>>(*r));
    EXPECT_EQ(std::get<std::vector<std::string>>(*r), expected);
  }
  static void ExpectNullopt(const OptionalResult& r) {
    EXPECT_FALSE(r.has_value());
  }
  static double ParseDistResult(const OptionalResult& r) {
    EXPECT_TRUE(r.has_value());
    if (!r.has_value()) return -1.0;
    EXPECT_TRUE(std::holds_alternative<ResString>(*r));
    if (!std::holds_alternative<ResString>(*r)) return -1.0;
    return std::stod(std::get<ResString>(*r).value);
  }

  void AddSicily() {
    Exec("GEOADD Sicily 13.361389 38.115556 Palermo");
    Exec("GEOADD Sicily 15.087269 37.502669 Catania");
  }
};

TEST_F(GeoTest, GEOADD_SinglePoint_Returns1) {
  ExpectInt(Exec("GEOADD key 0 0 a"), 1);
}

TEST_F(GeoTest, GEOADD_MultipleNewPoints_ReturnsCount) {
  ExpectInt(Exec("GEOADD key 0 0 a 1 0 b 2 0 c"), 3);
}

TEST_F(GeoTest, GEOADD_UpdateExisting_Returns0) {
  Exec("GEOADD key 0 0 a");
  ExpectInt(Exec("GEOADD key 1 1 a"), 0);
}

TEST_F(GeoTest, GEOADD_MixNewAndUpdated_ReturnsNewOnly) {
  Exec("GEOADD key 0 0 a");
  ExpectInt(Exec("GEOADD key 1 0 b 2 0 a"), 1);
}

TEST_F(GeoTest, GEOADD_CreatesKeyForNewIndex) {
  Exec("GEOADD key 0 0 a");
  ExpectInt(Exec("EXISTS key"), 1);
}

TEST_F(GeoTest, GEOADD_WrongType_ReturnsNullopt) {
  Exec("SET key str");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOADD key 0 0 a"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOADD_LonOutOfRange_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOADD key 200 0 a"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOADD_LatOutOfRange_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOADD key 0 90 a"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOADD_InvalidFloat_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOADD key notafloat 0 a"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOADD_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOADD key 0 0"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOADD_UnalignedArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOADD key 0 0 a 1"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOADD_OOM_ReturnsNullopt) {
  Exec("CONFIG SET maxmemory 1b");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOADD key 0 0 a"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEODIST_SamePoint_ReturnsZero) {
  Exec("GEOADD key 0 0 a");
  ExpectString(Exec("GEODIST key a a"), "0.0000");
}

TEST_F(GeoTest, GEODIST_DefaultUnit_IsMeters) {
  AddSicily();
  double dist = ParseDistResult(Exec("GEODIST Sicily Palermo Catania"));
  EXPECT_NEAR(dist, 166274.0, 500.0);
}

TEST_F(GeoTest, GEODIST_KmUnit) {
  AddSicily();
  double dist = ParseDistResult(Exec("GEODIST Sicily Palermo Catania km"));
  EXPECT_NEAR(dist, 166.274, 0.5);
}

TEST_F(GeoTest, GEODIST_MiUnit) {
  AddSicily();
  double dist = ParseDistResult(Exec("GEODIST Sicily Palermo Catania mi"));
  EXPECT_NEAR(dist, 103.3, 0.5);
}

TEST_F(GeoTest, GEODIST_FtUnit) {
  AddSicily();
  double dist = ParseDistResult(Exec("GEODIST Sicily Palermo Catania ft"));
  EXPECT_NEAR(dist, 545344.0, 2000.0);
}

TEST_F(GeoTest, GEODIST_CaseInsensitiveUnit) {
  AddSicily();
  double dist_lower = ParseDistResult(Exec("GEODIST Sicily Palermo Catania km"));
  double dist_upper = ParseDistResult(Exec("GEODIST Sicily Palermo Catania KM"));
  EXPECT_NEAR(dist_lower, dist_upper, 0.0001);
}

TEST_F(GeoTest, GEODIST_KeyNotExist_ReturnsNil) {
  ExpectNil(Exec("GEODIST missing a b"));
}

TEST_F(GeoTest, GEODIST_WrongType_ReturnsNil) {
  Exec("SET key str");
  ExpectNil(Exec("GEODIST key a b"));
}

TEST_F(GeoTest, GEODIST_Member1NotFound_ReturnsNil) {
  AddSicily();
  ExpectNil(Exec("GEODIST Sicily missing Catania"));
}

TEST_F(GeoTest, GEODIST_Member2NotFound_ReturnsNil) {
  AddSicily();
  ExpectNil(Exec("GEODIST Sicily Palermo missing"));
}

TEST_F(GeoTest, GEODIST_UnknownUnit_ReturnsNullopt) {
  AddSicily();
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEODIST Sicily Palermo Catania parsec"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEODIST_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEODIST key a"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEODIST_KmConsistentWithMeters) {
  AddSicily();
  double m  = ParseDistResult(Exec("GEODIST Sicily Palermo Catania m"));
  double km = ParseDistResult(Exec("GEODIST Sicily Palermo Catania km"));
  EXPECT_NEAR(m / 1000.0, km, 0.001);
}

TEST_F(GeoTest, GEOPOS_ExistingMember_ContainsCoords) {
  Exec("GEOADD key 0 0 a");
  ExpectStatus(Exec("GEOPOS key a"),
    "1) 1) \"0.000000\"\n   2) \"0.000000\"");
}

TEST_F(GeoTest, GEOPOS_MemberNotFound_ReturnsNil) {
  Exec("GEOADD key 0 0 a");
  ExpectStatus(Exec("GEOPOS key missing"), "1) (nil)");
}

TEST_F(GeoTest, GEOPOS_KeyNotExist_ReturnsNil) {
  ExpectStatus(Exec("GEOPOS missing a"), "1) (nil)");
}

TEST_F(GeoTest, GEOPOS_MultipleMembers_ContainsBothCoords) {
  Exec("GEOADD key 0 0 a");
  Exec("GEOADD key 1 0 b");
  auto r = Exec("GEOPOS key a b");
  ExpectStatusContains(r, "0.000000");
  ExpectStatusContains(r, "1.000000");
}

TEST_F(GeoTest, GEOPOS_MultipleMembersWithMissing_ShowsNilForMissing) {
  Exec("GEOADD key 0 0 a");
  auto r = Exec("GEOPOS key a missing");
  ExpectStatusContains(r, "0.000000");
  ExpectStatusContains(r, "(nil)");
}

TEST_F(GeoTest, GEOPOS_SicilyPalermo_ContainsLon) {
  AddSicily();
  ExpectStatusContains(Exec("GEOPOS Sicily Palermo"), "13.361389");
}

TEST_F(GeoTest, GEOPOS_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOPOS key"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOSEARCH_AllWithinRadius_ReturnsSortedAsc) {
  Exec("GEOADD key 0 0 near");
  Exec("GEOADD key 1 0 far");
  ExpectArray(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 200 km ASC"),
              {"near", "far"});
}

TEST_F(GeoTest, GEOSEARCH_FiltersByRadius) {
  Exec("GEOADD key 0 0 near");
  Exec("GEOADD key 10 0 far");
  ExpectArray(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 200 km"), {"near"});
}

TEST_F(GeoTest, GEOSEARCH_EmptyResult_NoneInRadius) {
  Exec("GEOADD key 10 0 a");
  ExpectArray(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 10 km"), {});
}

TEST_F(GeoTest, GEOSEARCH_KeyNotExist_ReturnsEmpty) {
  ExpectArray(Exec("GEOSEARCH missing FROMLONLAT 0 0 BYRADIUS 100 km"), {});
}

TEST_F(GeoTest, GEOSEARCH_WrongType_ReturnsEmpty) {
  Exec("SET key str");
  ExpectArray(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 100 km"), {});
}

TEST_F(GeoTest, GEOSEARCH_DescOrdering) {
  Exec("GEOADD key 0 0 near");
  Exec("GEOADD key 1 0 far");
  ExpectArray(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 200 km DESC"),
              {"far", "near"});
}

TEST_F(GeoTest, GEOSEARCH_CountLimitsResults) {
  Exec("GEOADD key 0 0 a");
  Exec("GEOADD key 1 0 b");
  Exec("GEOADD key 2 0 c");
  auto r = Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 500 km ASC COUNT 2");
  ASSERT_TRUE(r.has_value());
  ASSERT_TRUE(std::holds_alternative<std::vector<std::string>>(*r));
  EXPECT_EQ(std::get<std::vector<std::string>>(*r).size(), 2u);
}

TEST_F(GeoTest, GEOSEARCH_CountAscReturnsClosest) {
  Exec("GEOADD key 0 0 a");
  Exec("GEOADD key 1 0 b");
  Exec("GEOADD key 2 0 c");
  auto r = Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 500 km ASC COUNT 1");
  ASSERT_TRUE(r.has_value());
  ASSERT_TRUE(std::holds_alternative<std::vector<std::string>>(*r));
  EXPECT_EQ(std::get<std::vector<std::string>>(*r)[0], "a");
}

TEST_F(GeoTest, GEOSEARCH_KmUnit) {
  Exec("GEOADD key 0 0 a");
  ExpectArray(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 1 km"), {"a"});
}

TEST_F(GeoTest, GEOSEARCH_MetersUnit) {
  Exec("GEOADD key 0 0 a");
  ExpectArray(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 1000 m"), {"a"});
}

TEST_F(GeoTest, GEOSEARCH_CaseInsensitiveKeywords) {
  Exec("GEOADD key 0 0 a");
  ExpectArray(Exec("GEOSEARCH key fromlonlat 0 0 byradius 100 km"), {"a"});
}

TEST_F(GeoTest, GEOSEARCH_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOSEARCH_MissingUnit_ReturnsNullopt) {
  Exec("GEOADD key 0 0 a");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 100"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOSEARCH_UnsupportedUnit_ReturnsNullopt) {
  Exec("GEOADD key 0 0 a");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 100 parsec"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOSEARCH_InvalidCount_ReturnsNullopt) {
  Exec("GEOADD key 0 0 a");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 100 km COUNT 0"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOSEARCH_UnknownOption_ReturnsNullopt) {
  Exec("GEOADD key 0 0 a");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 100 km WITHSCORES"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOSEARCHSTORE_StoresResults_ReturnsCount) {
  Exec("GEOADD src 0 0 a");
  Exec("GEOADD src 1 0 b");
  ExpectInt(Exec("GEOSEARCHSTORE dest src FROMLONLAT 0 0 BYRADIUS 500 km"), 2);
}

TEST_F(GeoTest, GEOSEARCHSTORE_DestIsGeoType) {
  Exec("GEOADD src 0 0 a");
  Exec("GEOSEARCHSTORE dest src FROMLONLAT 0 0 BYRADIUS 100 km");
  ExpectStatus(Exec("TYPE dest"), "geospacial index");
}

TEST_F(GeoTest, GEOSEARCHSTORE_FiltersByRadius) {
  Exec("GEOADD src 0 0 near");
  Exec("GEOADD src 10 0 far");
  ExpectInt(Exec("GEOSEARCHSTORE dest src FROMLONLAT 0 0 BYRADIUS 200 km"), 1);
}

TEST_F(GeoTest, GEOSEARCHSTORE_StoredMembersSearchable) {
  Exec("GEOADD src 0 0 a");
  Exec("GEOSEARCHSTORE dest src FROMLONLAT 0 0 BYRADIUS 100 km");
  ExpectArray(Exec("GEOSEARCH dest FROMLONLAT 0 0 BYRADIUS 100 km"), {"a"});
}

TEST_F(GeoTest, GEOSEARCHSTORE_SourceNotExist_DeletesDest_Returns0) {
  Exec("SADD dest x");
  ExpectInt(Exec("GEOSEARCHSTORE dest missing FROMLONLAT 0 0 BYRADIUS 100 km"), 0);
  ExpectInt(Exec("EXISTS dest"), 0);
}

TEST_F(GeoTest, GEOSEARCHSTORE_CountLimitsStored) {
  Exec("GEOADD src 0 0 a");
  Exec("GEOADD src 1 0 b");
  Exec("GEOADD src 2 0 c");
  ExpectInt(Exec("GEOSEARCHSTORE dest src FROMLONLAT 0 0 BYRADIUS 500 km COUNT 2"), 2);
}

TEST_F(GeoTest, GEOSEARCHSTORE_TooFewArgs_ReturnsNullopt) {
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOSEARCHSTORE dest src FROMLONLAT 0 0 BYRADIUS"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOSEARCHSTORE_ParseError_ReturnsNullopt) {
  Exec("GEOADD src 0 0 a");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOSEARCHSTORE dest src FROMLONLAT 0 0 BYRADIUS 100 parsec"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOADD_BoundaryLon_Accepted) {
  ExpectInt(Exec("GEOADD key 179.999 0 east"), 1);
  ExpectInt(Exec("GEOADD key -179.999 0 west"), 1);
}

TEST_F(GeoTest, GEOADD_BoundaryLat_Accepted) {
  ExpectInt(Exec("GEOADD key 0 85 north"), 1);
  ExpectInt(Exec("GEOADD key 0 -85 south"), 1);
}

TEST_F(GeoTest, GEOADD_UpdateCoords_PosReflectedInGeopos) {
  Exec("GEOADD key 0 0 a");
  Exec("GEOADD key 10 20 a");
  ExpectStatusContains(Exec("GEOPOS key a"), "10.000000");
}

TEST_F(GeoTest, GEOADD_MultiplePointsSameKey_AllRetrievable) {
  Exec("GEOADD key 0 0 a 1 0 b 2 0 c");
  ExpectStatusContains(Exec("GEOPOS key a"), "0.000000");
  ExpectStatusContains(Exec("GEOPOS key b"), "1.000000");
  ExpectStatusContains(Exec("GEOPOS key c"), "2.000000");
}

TEST_F(GeoTest, GEODIST_Symmetric) {
  AddSicily();
  double ab = ParseDistResult(Exec("GEODIST Sicily Palermo Catania km"));
  double ba = ParseDistResult(Exec("GEODIST Sicily Catania Palermo km"));
  EXPECT_NEAR(ab, ba, 0.0001);
}

TEST_F(GeoTest, GEODIST_MeterToKmRatio) {
  AddSicily();
  double m  = ParseDistResult(Exec("GEODIST Sicily Palermo Catania m"));
  double km = ParseDistResult(Exec("GEODIST Sicily Palermo Catania km"));
  EXPECT_NEAR(m, km * 1000.0, 1.0);
}

TEST_F(GeoTest, GEODIST_AfterUpdate_ReflectsNewCoords) {
  Exec("GEOADD key 0 0 a");
  Exec("GEOADD key 0 0 b");
  double before = ParseDistResult(Exec("GEODIST key a b"));
  Exec("GEOADD key 10 0 b");
  double after = ParseDistResult(Exec("GEODIST key a b"));
  EXPECT_NEAR(before, 0.0, 0.001);
  EXPECT_GT(after, 0.0);
}

TEST_F(GeoTest, GEOSEARCH_MiUnit) {
  Exec("GEOADD key 0 0 a");
  ExpectArray(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 1 mi"), {"a"});
}

TEST_F(GeoTest, GEOSEARCH_FtUnit) {
  Exec("GEOADD key 0 0 a");
  ExpectArray(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 5000 ft"), {"a"});
}

TEST_F(GeoTest, GEOSEARCH_PointExactlyOnCenter_Included) {
  Exec("GEOADD key 0 0 a");
  ExpectArray(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 1 m"), {"a"});
}

TEST_F(GeoTest, GEOSEARCH_CountZero_ReturnsNullopt) {
  Exec("GEOADD key 0 0 a");
  testing::internal::CaptureStderr();
  ExpectNullopt(Exec("GEOSEARCH key FROMLONLAT 0 0 BYRADIUS 100 km COUNT 0"));
  testing::internal::GetCapturedStderr();
}

TEST_F(GeoTest, GEOSEARCHSTORE_OverwriteExistingGeoKey) {
  Exec("GEOADD src 0 0 a 1 0 b");
  Exec("GEOSEARCHSTORE dest src FROMLONLAT 0 0 BYRADIUS 500 km");
  Exec("GEOADD src 2 0 c");
  ExpectInt(Exec("GEOSEARCHSTORE dest src FROMLONLAT 0 0 BYRADIUS 500 km"), 3);
  ExpectStatus(Exec("TYPE dest"), "geospacial index");
}

TEST_F(GeoTest, GEOSEARCHSTORE_EmptyResult_DestDeleted) {
  Exec("GEOADD src 0 0 a");
  Exec("GEOSEARCHSTORE dest src FROMLONLAT 0 0 BYRADIUS 100 km");
  ExpectInt(Exec("GEOSEARCHSTORE dest src FROMLONLAT 100 0 BYRADIUS 1 km"), 0);
  ExpectInt(Exec("EXISTS dest"), 0);
}

TEST_F(GeoTest, EXPIRE_OnGeoKey_Returns1) {
  Exec("GEOADD geo 0 0 a");
  ExpectInt(Exec("EXPIRE geo 60"), 1);
}

TEST_F(GeoTest, TTL_OnGeoKey_NoExpiry_ReturnsMinus1) {
  Exec("GEOADD geo 0 0 a");
  ExpectInt(Exec("TTL geo"), static_cast<size_t>(-1));
}

TEST_F(GeoTest, TTL_OnGeoKey_AfterExpire_ReturnsPositive) {
  Exec("GEOADD geo 0 0 a");
  Exec("EXPIRE geo 60");
  auto r = Exec("TTL geo");
  ASSERT_TRUE(r.has_value());
  ASSERT_TRUE(std::holds_alternative<size_t>(*r));
  EXPECT_GT(std::get<size_t>(*r), 0u);
}

TEST_F(GeoTest, DEL_GeoKey_RemovesKey) {
  Exec("GEOADD geo 0 0 a");
  ExpectInt(Exec("DEL geo"), 1);
  ExpectInt(Exec("EXISTS geo"), 0);
}

TEST_F(GeoTest, TYPE_GeoKey_ReturnsGeospacialIndex) {
  Exec("GEOADD key 0 0 a");
  ExpectStatus(Exec("TYPE key"), "geospacial index");
}
