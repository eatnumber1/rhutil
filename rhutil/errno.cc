#include "rhutil/errno.h"

#include <cerrno>
#include <cstring>

#include "rhutil/status.h"

namespace rhutil {

Status ErrnoAsStatus() {
  return ErrnoAsStatus(errno);
}

Status ErrnoAsStatus(int err) {
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

}  // namespace rhutil
