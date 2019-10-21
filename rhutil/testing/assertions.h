#ifndef RHUTIL_TESTING_ASSERTIONS_H_
#define RHUTIL_TESTING_ASSERTIONS_H_

#include "gtest/gtest.h"
#include "rhutil/status.h"

namespace rhutil {

testing::AssertionResult IsOk(const Status &);

template <typename T>
testing::AssertionResult IsOk(const StatusOr<T> &st) {
  return IsOk(st.status());
}

}  // namespace rhutil

#endif  // RHUTIL_TESTING_ASSERTIONS_H_
