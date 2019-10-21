#include "rhutil/testing/assertions.h"

#include <string>

namespace rhutil {

using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;

AssertionResult IsOk(const Status &st) {
  if (st.ok()) {
    return AssertionSuccess() << "No error occurred";
  } else {
    return AssertionFailure() << st;
  }
}

}  // namespace rhutil
