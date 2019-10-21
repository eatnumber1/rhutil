#ifndef CDMANIP_UTIL_CLEANUP_H_
#define CDMANIP_UTIL_CLEANUP_H_

#include <functional>

namespace cdmanip {

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

}  // namespace cdmanip

#endif  // CDMANIP_UTIL_CLEANUP_H_
