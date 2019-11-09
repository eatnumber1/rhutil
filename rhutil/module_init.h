#ifndef RHUTIL_MODULE_INIT_H_
#define RHUTIL_MODULE_INIT_H_

#include <vector>

#include "absl/synchronization/mutex.h"

namespace rhutil {

class ModuleInit {
 public:
  using Initializer = void();

  explicit ModuleInit(Initializer *init);

  static void InitializeAll();

 private:
  static std::vector<Initializer*> *Initializers();

  static absl::Mutex initializers_mu_;
};

}  // namespace rhutil

#endif  // RHUTIL_MODULE_INIT_H_
