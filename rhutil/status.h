#ifndef RHUTIL_STATUS_H_
#define RHUTIL_STATUS_H_

#include <string>
#include <string_view>
#include <utility>
#include <ostream>
#include <sstream>

#include "absl/base/attributes.h"
#include "absl/strings/str_cat.h"

#define RETURN_IF_ERROR(expr) \
  if (::rhutil::Status s = (expr); !s.ok()) return ::rhutil::StatusBuilder(s)

#define CHECK_OK(expr) \
    do { if (auto s = (expr); !s.ok()) StatusInternalOnlyDie(s); } while (false)
#define CHECK(expr) \
    do { \
      if (!(expr)) StatusInternalOnlyDie(::rhutil::InternalError(#expr)); \
    } while (false)

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

std::ostream &operator<<(std::ostream &, const Status &);

class ABSL_MUST_USE_RESULT StatusBuilder {
 public:
   StatusBuilder(const Status &original);

   operator Status() const;

   ABSL_MUST_USE_RESULT bool ok() const;

   template <typename T>
   StatusBuilder &operator<<(const T &value);

 private:
   Status status_;
};

template <typename T>
class ABSL_MUST_USE_RESULT StatusOr {
 public:
  explicit StatusOr();

  StatusOr(T data);
  StatusOr(Status);
  StatusOr(StatusBuilder builder);

  const T &ValueOrDie() const &;
  T &&ValueOrDie() &&;

  const Status &status() const;
  bool ok() const;

 private:
  Status status_;
  T data_;
};

std::ostream& operator<<(std::ostream& os, const StatusBuilder &builder);

Status OkStatus();
Status AbortedError(std::string_view msg);
Status CancelledError(std::string_view msg);
Status DeadlineExceededError(std::string_view msg);
Status FailedPreconditionError(std::string_view msg);
Status InternalError(std::string_view msg);
Status InvalidArgumentError(std::string_view msg);
Status NotFoundError(std::string_view msg);
Status OutOfRangeError(std::string_view msg);
Status PermissionDeniedError(std::string_view msg);
Status ResourceExhaustedError(std::string_view msg);
Status UnauthenticatedError(std::string_view msg);
Status UnavailableError(std::string_view msg);
Status UnimplementedError(std::string_view msg);
Status UnknownError(std::string_view msg);

StatusBuilder UnknownErrorBuilder();
StatusBuilder InvalidArgumentErrorBuilder();
StatusBuilder UnimplementedErrorBuilder();
StatusBuilder InternalErrorBuilder();
StatusBuilder FailedPreconditionErrorBuilder();
StatusBuilder NotFoundErrorBuilder();

bool IsFailedPrecondition(const Status &st);
bool IsNotFound(const Status &st);

// implementation details below

[[noreturn]]
void StatusInternalOnlyDie(const Status &);

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
StatusOr<T>::StatusOr(StatusBuilder builder)
  : status_(std::move(builder))
  {}

template <typename T>
const T &StatusOr<T>::ValueOrDie() const & {
  CHECK_OK(status_);
  return data_;
}

template <typename T>
T &&StatusOr<T>::ValueOrDie() && {
  CHECK_OK(status_);
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

template <typename T>
StatusBuilder &StatusBuilder::operator<<(const T &value) {
  if (status_.ok()) return *this;
  std::ostringstream strm;
  strm << value;
  status_ = Status(status_.code(), absl::StrCat(status_.message(), strm.str()));
  return *this;
}

}  // namespace rhutil

#endif  // RHUTIL_STATUS_H_
