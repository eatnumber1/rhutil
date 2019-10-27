#ifndef RHUTIL_TESTING_PROTOBUF_ASSERTIONS_H_
#define RHUTIL_TESTING_PROTOBUF_ASSERTIONS_H_

#include "gtest/gtest.h"
#include "google/protobuf/util/message_differencer.h"
#include "google/protobuf/message.h"

namespace rhutil {

testing::AssertionResult IsEqual(const google::protobuf::Message &a,
                                 const google::protobuf::Message &b);

}  // namespace rhutil

#endif  // RHUTIL_TESTING_PROTOBUF_ASSERTIONS_H_
