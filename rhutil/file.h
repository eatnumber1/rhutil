#ifndef RHUTIL_FILE_H_
#define RHUTIL_FILE_H_

#include <fstream>
#include <ios>
#include <string_view>

#include "rhutil/status.h"

namespace rhutil {

StatusOr<std::ifstream> OpenInputFile(std::string_view path);
StatusOr<std::ifstream> OpenInputFile(std::string_view path,
                                      std::ios_base::openmode mode);

}  // namespace rhutil

#endif  // RHUTIL_FILE_H_
