#include "rhutil/status.h"

#include "gtest/gtest.h"

namespace rhutil {
namespace {

TEST(StatusTest, SimpleString) {
  Status s(StatusCode::kUnknown, "Unknown error");
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), StatusCode::kUnknown);
  EXPECT_EQ(s.ToString(), "UNKNOWN: Unknown error");
}

TEST(StatusTest, Ok) {
  Status s;
  EXPECT_TRUE(s.ok());
  EXPECT_EQ(s.ToString(), "OK");

  Status s2(StatusCode::kOk, "Ignored");
  EXPECT_TRUE(s2.ok());
  EXPECT_EQ(s2.ToString(), "OK");

  Status s3 = OkStatus();
  EXPECT_TRUE(s2.ok());
  EXPECT_EQ(s2.ToString(), "OK");
}

TEST(StatusOrTest, Ok) {
  auto OkInt = []() -> StatusOr<int> {
    return 5;
  };

  StatusOr<int> int_or = OkInt();

  ASSERT_TRUE(int_or.ok());
  EXPECT_EQ(int_or.ValueOrDie(), 5);
}

TEST(StatusOrTest, Bad) {
  auto Error = []() -> StatusOr<int> {
    return UnknownError("error");
  };

  StatusOr<int> int_or = Error();

  ASSERT_FALSE(int_or.ok());
  EXPECT_EQ(int_or.status().code(), StatusCode::kUnknown);
  EXPECT_EQ(int_or.status().ToString(), "UNKNOWN: error");
}

TEST(StatusBuilderTest, Ok) {
  StatusBuilder sb(OkStatus());
  sb << "an error occurred";
  ASSERT_TRUE(sb.ok());

  Status s(sb);
  EXPECT_EQ(s.code(), StatusCode::kOk);
  EXPECT_EQ(s.ToString(), "OK");
}

TEST(StatusBuilderTest, Bad) {
  StatusBuilder sb(UnknownError("error"));
  sb << " an error occurred";
  ASSERT_FALSE(sb.ok());

  Status s(sb);
  EXPECT_EQ(s.code(), StatusCode::kUnknown);
  EXPECT_EQ(s.ToString(), "UNKNOWN: error an error occurred");
}

}  // namespace
}  // namespace rhutil
