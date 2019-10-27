#include "rhutil/file.h"

#include <cassert>

#include "rhutil/errno.h"
#include "absl/strings/str_format.h"

namespace rhutil {

StatusOr<std::ifstream> OpenInputFile(std::string_view path) {
  return OpenInputFile(path, std::ios::in);
}

StatusOr<std::ifstream> OpenInputFile(std::string_view path,
                                      std::ios_base::openmode mode) {
  std::ifstream istrm;
  istrm.open(std::string(path), mode);
  if (istrm.fail()) {
    return StatusBuilder(ErrnoAsStatus()) << "Failed to open " << path;
  }
  assert(istrm.is_open());
  return std::move(istrm);
}

}  // namespace rhutil
