#include "rhutil/cleanup.h"

#include <utility>

namespace rhutil {

Cleanup::Cleanup() : released_(true) {};

Cleanup::Cleanup(std::function<void()> func)
  : released_(false), func_(std::move(func)) {}

Cleanup::~Cleanup() {
  if (!released_) func_();
}

Cleanup::Cleanup(Cleanup &&o) {
  o.released_ = true;
  func_ = std::move(o.func_);
}

}  // namespace rhutil
