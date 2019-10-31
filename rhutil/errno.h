#ifndef RHUTIL_ERRNO_H_
#define RHUTIL_ERRNO_H_

#include "rhutil/status.h"

namespace rhutil {

Status ErrnoAsStatus();
Status ErrnoAsStatus(int err);

}  // namespace rhutil

#endif  // RHUTIL_ERRNO_H_
