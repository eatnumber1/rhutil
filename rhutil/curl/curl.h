#ifndef RHUTIL_CURL_CURL_H_
#define RHUTIL_CURL_CURL_H_

#include <memory>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string_view>
#include <functional>

#include "rhutil/status.h"
#include "curl/curl.h"
#include "absl/types/span.h"
#include "absl/synchronization/mutex.h"
#include "absl/container/flat_hash_map.h"

namespace rhutil {

// Must not be used on handles that were not created by CurlEasyInit.
class CurlHandleDeleter {
 public:
  void operator()(CURL *);
};

class CurlSListDeleter {
 public:
  void operator()(curl_slist *);
};

class CurlStrDeleter {
 public:
  void operator()(char *);
};

class CurlShareDeleter {
 public:
  void operator()(CURLSH *);
};

class ThreadSafeCurlShare {
 public:
  ThreadSafeCurlShare();

  CURLSH *ptr() const;

 private:
  void Lock(CURL*, curl_lock_data, curl_lock_access) NO_THREAD_SAFETY_ANALYSIS;
  void Unlock(CURL*, curl_lock_data) NO_THREAD_SAFETY_ANALYSIS;

  static void Lock(CURL*, curl_lock_data, curl_lock_access, void*);
  static void Unlock(CURL*, curl_lock_data, void*);

  class MutexLock {
   public:
    virtual ~MutexLock() = 0;
  };
  class SharedMutexLock;
  class SingleMutexLock;

  static absl::flat_hash_map<curl_lock_data, std::unique_ptr<MutexLock>>
      CreateLocksMap();
  static absl::flat_hash_map<curl_lock_data, std::unique_ptr<absl::Mutex>>
      CreateMutexesMap();

  std::unique_ptr<CURLSH, CurlShareDeleter> share_;
  // Do not modify the size or keys of this map.
  absl::flat_hash_map<curl_lock_data, std::unique_ptr<absl::Mutex>> mutexes_;
  // This map's values are protected by the mutexes in mutexes_. Do not modify
  // the size or keys of this map.
  absl::flat_hash_map<curl_lock_data, std::unique_ptr<MutexLock>> locks_;
};

// All string_views used herein must be null-terminated.
class CurlURL {
 public:
  CurlURL();
  ~CurlURL();
  explicit CurlURL(CURLU *url);

  static CurlURL FromStringOrDie(std::string_view url);
  static StatusOr<CurlURL> FromString(std::string_view url);

  CurlURL(const CurlURL &);
  CurlURL(CurlURL &&);
  CurlURL &operator=(CurlURL);

  friend void swap(CurlURL &, CurlURL &);

  CURLU *ReleaseCURLU();
  CURLU *GetCURLU() const;

  Status SetURL(std::string_view url);
  void SetUser(std::string_view user);
  void SetPassword(std::string_view password);
  void SetHost(std::string_view host);
  void SetPort(uint16_t port);
  void SetPath(std::string_view path);
  void SetScheme(std::string_view scheme);

  std::unique_ptr<char, CurlStrDeleter> GetURL() const;
  std::unique_ptr<char, CurlStrDeleter> GetScheme() const;
  std::unique_ptr<char, CurlStrDeleter> GetUser() const;
  std::unique_ptr<char, CurlStrDeleter> GetPassword() const;
  std::unique_ptr<char, CurlStrDeleter> GetHost() const;
  std::unique_ptr<char, CurlStrDeleter> GetPath() const;
  uint16_t GetPort() const;

  StatusOr<std::unique_ptr<char, CurlStrDeleter>> GetAndMapErrorToNull(
      CURLUPart part, CURLUcode nullcode, unsigned int flags = 0) const;

  Status Set(CURLUPart part, std::string_view content, unsigned int flags = 0);
  StatusOr<std::unique_ptr<char, CurlStrDeleter>> Get(
      CURLUPart part, unsigned int flags = 0) const;

  static Status CodeToStatus(CURLUcode code);

 private:
   CURLU *url_;
};

bool AbslParseFlag(absl::string_view text, CurlURL *url, std::string *error);

std::string AbslUnparseFlag(CurlURL url);

std::unique_ptr<CURL, CurlHandleDeleter> CurlEasyInit();

template <typename... Parameters>
Status CurlEasySetopt(CURL *handle, CURLoption option, Parameters... params);
template <typename... Parameters>
Status CurlEasyGetInfo(CURL *handle, CURLINFO info, Parameters... params);

template <typename... Parameters>
Status CurlShareSetopt(CURLSH *share, CURLSHoption option,
                       Parameters... params);

Status CurlEasySetWriteCallback(
    CURL *handle, std::function<Status(std::string_view, size_t*)> callback);

// The passed-in string_views must be null-terminated.
std::unique_ptr<curl_slist, CurlSListDeleter> NewCurlSList(
    absl::Span<const std::string_view> strings);

Status CurlEasyPerform(CURL *handle);

// Must be called before any other threads are created.
Status CurlGlobalInit();

Status CurlCodeToStatus(CURLcode code);
Status CurlCodeToStatus(CURLcode code, CURL *handle);
Status CurlShareCodeToStatus(CURLSHcode code);

Status HTTPCodeToStatus(int http_code);

// implementation details below

template <typename... Parameters>
Status CurlEasySetopt(CURL *handle, CURLoption option, Parameters... params) {
  CURLcode code = curl_easy_setopt(handle, option,
                                   std::forward<Parameters>(params)...);
  return CurlCodeToStatus(code, handle);
}
template <typename... Parameters>
Status CurlEasyGetInfo(CURL *handle, CURLINFO info, Parameters... params) {
  CURLcode code = curl_easy_getinfo(handle, info,
                                    std::forward<Parameters>(params)...);
  return CurlCodeToStatus(code, handle);
}

template <typename... Parameters>
Status CurlShareSetopt(CURLSH *share, CURLSHoption option,
                       Parameters... params) {
  CURLSHcode code = curl_share_setopt(share, option,
                                      std::forward<Parameters>(params)...);
  return CurlShareCodeToStatus(code);
}

}  // namespace rhutil

#endif  // RHUTIL_CURL_CURL_H_
