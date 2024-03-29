#ifndef RHUTIL_CLEANUP_H_
#define RHUTIL_CLEANUP_H_

#include <functional>

namespace rhutil {

class Cleanup {
 public:
  Cleanup();
  Cleanup(std::function<void()> func);
  ~Cleanup();

  Cleanup(Cleanup &&o);
  Cleanup(const Cleanup &o) = delete;

 private:
  bool released_;
  std::function<void()> func_;
};

}  // namespace rhutil

#endif  // RHUTIL_CLEANUP_H_
