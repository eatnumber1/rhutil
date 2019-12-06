#include "rhutil/status.h"

#include <type_traits>
#include <iostream>
#include <boost/stacktrace.hpp>

#include "absl/strings/str_format.h"

namespace rhutil {

std::string StatusCodeToString(StatusCode sc) {
  switch (sc) {
    case StatusCode::kOk:
      return "OK";
    case StatusCode::kCancelled:
      return "CANCELLED";
    case StatusCode::kUnknown:
      return "UNKNOWN";
    case StatusCode::kInvalidArgument:
      return "INVALID_ARGUMENT";
    case StatusCode::kDeadlineExceeded:
      return "DEADLINE_EXCEEDED";
    case StatusCode::kNotFound:
      return "NOT_FOUND";
    case StatusCode::kAlreadyExists:
      return "ALREADY_EXISTS";
    case StatusCode::kPermissionDenied:
      return "PERMISSION_DENIED";
    case StatusCode::kResourceExhausted:
      return "RESOURCE_EXHAUSTED";
    case StatusCode::kFailedPrecondition:
      return "FAILED_PRECONDITION";
    case StatusCode::kAborted:
      return "ABORTED";
    case StatusCode::kOutOfRange:
      return "OUT_OF_RANGE";
    case StatusCode::kUnimplemented:
      return "UNIMPLEMENTED";
    case StatusCode::kInternal:
      return "INTERNAL";
    case StatusCode::kUnavailable:
      return "UNAVAILABLE";
    case StatusCode::kDataLoss:
      return "DATA_LOSS";
    case StatusCode::kUnauthenticated:
      return "UNAUTHENTICATED";
    default:
      return "INVALID_STATUS_CODE";
  }
}

std::ostream &operator<<(std::ostream &os, StatusCode sc) {
  return os << StatusCodeToString(sc);
}

Status::Status(StatusCode code, std::string_view msg)
  : code_(code), message_(std::string(msg))
  {}

bool Status::ok() const {
  return code_ == StatusCode::kOk;
}

void Status::Update(Status &&s) {
  if (!ok()) return;
  *this = std::move(s);
}

void Status::Update(const Status &s) {
  if (!ok()) return;
  *this = s;
}

StatusCode Status::code() const {
  return code_;
}

std::string_view Status::message() const {
  return message_;
}

std::string Status::ToString() const {
  StatusCode code = this->code();
  std::string code_str = StatusCodeToString(code);
  if (code == StatusCode::kOk) return code_str;
  return absl::StrFormat("%s: %s", std::move(code_str), message_);
}

void Status::IgnoreError() const {}

Status OkStatus() { return {}; }

Status AbortedError(std::string_view msg) {
  return {StatusCode::kAborted, msg};
}
Status CancelledError(std::string_view msg) {
  return {StatusCode::kCancelled, msg};
}
Status DeadlineExceededError(std::string_view msg) {
  return {StatusCode::kDeadlineExceeded, msg};
}
Status FailedPreconditionError(std::string_view msg) {
  return {StatusCode::kFailedPrecondition, msg};
}
Status InternalError(std::string_view msg) {
  return {StatusCode::kInternal, msg};
}
Status InvalidArgumentError(std::string_view msg) {
  return {StatusCode::kInvalidArgument, msg};
}
Status NotFoundError(std::string_view msg) {
  return {StatusCode::kNotFound, msg};
}
Status OutOfRangeError(std::string_view msg) {
  return {StatusCode::kOutOfRange, msg};
}
Status PermissionDeniedError(std::string_view msg) {
  return {StatusCode::kPermissionDenied, msg};
}
Status ResourceExhaustedError(std::string_view msg) {
  return {StatusCode::kResourceExhausted, msg};
}
Status UnauthenticatedError(std::string_view msg) {
  return {StatusCode::kUnauthenticated, msg};
}
Status UnavailableError(std::string_view msg) {
  return {StatusCode::kUnavailable, msg};
}
Status UnimplementedError(std::string_view msg) {
  return {StatusCode::kUnimplemented, msg};
}
Status UnknownError(std::string_view msg) {
  return {StatusCode::kUnknown, msg};
}

StatusBuilder UnknownErrorBuilder() {
  return {UnknownError("")};
}
StatusBuilder InvalidArgumentErrorBuilder() {
  return {InvalidArgumentError("")};
}
StatusBuilder UnimplementedErrorBuilder() {
  return {UnimplementedError("")};
}
StatusBuilder InternalErrorBuilder() {
  return {InternalError("")};
}
StatusBuilder FailedPreconditionErrorBuilder() {
  return {FailedPreconditionError("")};
}
StatusBuilder NotFoundErrorBuilder() {
  return {NotFoundError("")};
}

bool IsFailedPrecondition(const Status &st) {
  return st.code() == StatusCode::kFailedPrecondition;
}

std::ostream &operator<<(std::ostream &o, const Status &s) {
  return o << s.ToString();
}

void StatusInternalOnlyDie(const Status &st) {
  std::cerr << st << std::endl;
  std::cerr << boost::stacktrace::stacktrace() << std::endl;
  std::abort();
}

StatusBuilder::StatusBuilder(const Status &original)
  : status_(original)
  {}

StatusBuilder::operator Status() const {
  return status_;
}

std::ostream &operator<<(std::ostream &os, const StatusBuilder &builder) {
  return os << static_cast<Status>(builder);
}

bool StatusBuilder::ok() const {
  return status_.ok();
}

}  // namespace rhutil
