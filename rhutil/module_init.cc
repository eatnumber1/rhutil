#include "rhutil/module_init.h"

namespace rhutil {

using Initializer = ::rhutil::ModuleInit::Initializer;

ABSL_CONST_INIT absl::Mutex ModuleInit::initializers_mu_(absl::kConstInit);

ModuleInit::ModuleInit(Initializer *init) {
  std::vector<Initializer*> *initializers = Initializers();
  absl::MutexLock lock(&initializers_mu_);
  initializers->emplace_back(init);
}

std::vector<Initializer*> *ModuleInit::Initializers() {
  static std::vector<Initializer*> *initializers = []() {
    return new std::vector<Initializer*>();
  }();
  return initializers;
}

void ModuleInit::InitializeAll() {
  initializers_mu_.AssertNotHeld();
  for (Initializer *init : *Initializers()) {
    init();
  }
}

}  // namespace rhutil
