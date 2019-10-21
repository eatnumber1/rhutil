#ifndef RHUTIL_UTIL_STATUS_H_
#define RHUTIL_UTIL_STATUS_H_

#include <string>
#include <string_view>
#include <utility>
#include <ostream>

#include "absl/base/attributes.h"

#define RETURN_IF_ERROR(expr) \
  if (::rhutil::Status s = (expr); !s.ok()) return s

#define CHECK_OK(expr) StatusInternalOnlyDieIfNotOk(expr)

#define ASSIGN_OR_RETURN(decl, expr) \
  decl = ({ \
    auto st = (expr); \
    if (!st.ok()) { return std::move(st).status(); } \
    std::move(st).ValueOrDie(); \
  })

namespace rhutil {

enum class StatusCode : int {
  kOk = 0,
  kCancelled = 1,
  kUnknown = 2,
  kInvalidArgument = 3,
  kDeadlineExceeded = 4,
  kNotFound = 5,
  kAlreadyExists = 6,
  kPermissionDenied = 7,
  kResourceExhausted = 8,
  kFailedPrecondition = 9,
  kAborted = 10,
  kOutOfRange = 11,
  kUnimplemented = 12,
  kInternal = 13,
  kUnavailable = 14,
  kDataLoss = 15,
  kUnauthenticated = 16,
  kDoNotUseReservedForFutureExpansionUseDefaultInSwitchInstead_ = 20
};

std::string StatusCodeToString(StatusCode sc);
std::ostream &operator<<(std::ostream &, StatusCode);

class ABSL_MUST_USE_RESULT Status final {
 public:
  Status() = default;

  Status(StatusCode code, std::string_view msg);

  ABSL_MUST_USE_RESULT bool ok() const;
  ABSL_MUST_USE_RESULT std::string ToString() const;
  ABSL_MUST_USE_RESULT StatusCode code() const;
  ABSL_MUST_USE_RESULT std::string_view message() const;

  void Update(const Status &);
  void Update(Status &&);

  void IgnoreError() const;

  friend void swap(Status &a, Status &b);

 private:
  StatusCode code_ = StatusCode::kOk;
  std::string message_;
};

Status OkStatus();
Status UnknownError(std::string_view msg);
Status InvalidArgumentError(std::string_view msg);
Status UnimplementedError(std::string_view msg);
Status InternalError(std::string_view msg);

std::ostream &operator<<(std::ostream &, const Status &);

template <typename T>
class ABSL_MUST_USE_RESULT StatusOr {
 public:
  explicit StatusOr();

  StatusOr(T data);
  StatusOr(Status);

  const T &ValueOrDie() const &;
  T &&ValueOrDie() &&;

  const Status &status() const;
  bool ok() const;

 private:
  Status status_;
  T data_;
};

// implementation details below

void StatusInternalOnlyDieIfNotOk(const Status &);

template <typename T>
StatusOr<T>::StatusOr()
  : status_(StatusCode::kUnknown, "")
  {}

template <typename T>
StatusOr<T>::StatusOr(T data)
  : data_(std::move(data))
  {}

template <typename T>
StatusOr<T>::StatusOr(Status s)
  : status_(std::move(s))
  {}

template <typename T>
const T &StatusOr<T>::ValueOrDie() const & {
  StatusInternalOnlyDieIfNotOk(status_);
  return data_;
}

template <typename T>
T &&StatusOr<T>::ValueOrDie() && {
  StatusInternalOnlyDieIfNotOk(status_);
  return std::move(data_);
}

template <typename T>
const Status &StatusOr<T>::status() const {
  return status_;
}

template <typename T>
bool StatusOr<T>::ok() const {
  return status_.ok();
}

}  // namespace rhutil

#endif  // RHUTIL_UTIL_STATUS_H_
