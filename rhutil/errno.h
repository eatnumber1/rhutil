#ifndef RHUTIL_ERRNO_H_
#define RHUTIL_ERRNO_H_

#include <system_error>

#include "rhutil/status.h"

namespace rhutil {

Status ErrnoAsStatus();
Status ErrnoAsStatus(int err);

Status ErrorCodeAsStatus(std::error_code code);

Status ProcessExitCodeToStatus(int code);

}  // namespace rhutil

#endif  // RHUTIL_ERRNO_H_
