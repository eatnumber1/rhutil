#include "rhutil/errno.h"

#include <cerrno>
#include <cstring>

#include "rhutil/status.h"

namespace rhutil {

Status ErrnoAsStatus() {
  return ErrnoAsStatus(errno);
}

Status ErrnoAsStatus(int err) {
  if (err == 0) return OkStatus();
  auto code = StatusCode::kUnknown;
  switch (err) {
    case ENOENT:
      code = StatusCode::kNotFound;
      break;
    case EINVAL:
      code = StatusCode::kInvalidArgument;
    // TODO(eatnumber1): Fill in more errors.
    default:
      break;
  }
  return {code, std::strerror(err)};
}

Status ErrorCodeAsStatus(std::error_code code) {
  if (!code) return OkStatus();
  const std::error_category &cat = code.category();
  if (cat == std::generic_category()) {
    return ErrnoAsStatus(code.value());
  }
  return StatusBuilder({StatusCode::kUnknown, ""})
      << code << ": " << code.message();
}

Status ProcessExitCodeToStatus(int code) {
  if (code == 0) return OkStatus();
  return UnknownErrorBuilder() << "code " << code;
}

}  // namespace rhutil
