#include "rhutil/curl/curl.h"

#include <cstddef>
#include <cstdio>
#include <string>
#include <memory>
#include <string_view>

#include "rhutil/errno.h"

namespace rhutil {
namespace {

struct CurlHandlePrivate {
  std::function<Status(std::string_view, size_t*)> write_callback;
  Status last_write_error;
  char error_buffer[CURL_ERROR_SIZE] = { '\0' };
};

CurlHandlePrivate *GetPrivate(CURL *handle) {
  CurlHandlePrivate *priv = nullptr;
  CURLcode code = curl_easy_getinfo(handle, CURLINFO_PRIVATE, &priv);
  CHECK_OK(CurlCodeToStatus(code));
  CHECK(priv != nullptr);
  return priv;
}

void SetPrivate(CURL *handle, CurlHandlePrivate *priv) {
  CHECK_OK(CurlCodeToStatus(curl_easy_setopt(handle, CURLOPT_PRIVATE, priv)));
}

size_t CurlWriteCallback(char *ptr, size_t, size_t nmemb, void *userdata) {
  auto *priv = reinterpret_cast<CurlHandlePrivate*>(userdata);
  size_t flags = 0;
  Status err = priv->write_callback({ptr, nmemb}, &flags);
  if (!err.ok()) priv->last_write_error = err;
  if (flags != 0) return flags;
  const size_t err_rc = nmemb == 0 ? 1 : 0;
  return err.ok() ? nmemb : err_rc;
}

constexpr absl::Span<const curl_lock_data> GetAllCurlLockData() {
  constexpr curl_lock_data all_curl_lock_data[] = {
    CURL_LOCK_DATA_NONE,
    CURL_LOCK_DATA_SHARE,
    CURL_LOCK_DATA_COOKIE,
    CURL_LOCK_DATA_DNS,
    CURL_LOCK_DATA_SSL_SESSION,
    CURL_LOCK_DATA_CONNECT,
    CURL_LOCK_DATA_PSL
  };
  constexpr size_t num_curl_lock_data =
      sizeof(all_curl_lock_data) / sizeof(curl_lock_data);
  static_assert(
      num_curl_lock_data == CURL_LOCK_DATA_LAST,
      "all_curl_lock_data does not cover all possible lockable data in this "
      "version of CURL");

  return {all_curl_lock_data, num_curl_lock_data};
}

}  // namespace

absl::flat_hash_map<curl_lock_data, std::unique_ptr<absl::Mutex>>
ThreadSafeCurlShare::CreateMutexesMap() {
  absl::flat_hash_map<curl_lock_data, std::unique_ptr<absl::Mutex>> mutexes;
  for (curl_lock_data lock_data : GetAllCurlLockData()) {
    mutexes.emplace(lock_data, std::make_unique<absl::Mutex>());
  }
  return mutexes;
}

absl::flat_hash_map<curl_lock_data,
                    std::unique_ptr<ThreadSafeCurlShare::MutexLock>>
ThreadSafeCurlShare::CreateLocksMap() {
  absl::flat_hash_map<curl_lock_data,
                      std::unique_ptr<ThreadSafeCurlShare::MutexLock>> locks;
  for (curl_lock_data lock_data : GetAllCurlLockData()) {
    locks.emplace(lock_data, nullptr);
  }
  return locks;
}

void CurlHandleDeleter::operator()(CURL *handle) {
  delete GetPrivate(handle);
  curl_easy_cleanup(handle);
}

void CurlSListDeleter::operator()(curl_slist *list) {
  curl_slist_free_all(list);
}

void CurlURLDeleter::operator()(CURLU *url) {
  curl_url_cleanup(url);
}

void CurlStrDeleter::operator()(char *ptr) {
  curl_free(ptr);
}

void CurlShareDeleter::operator()(CURLSH *share) {
  CHECK_OK(CurlShareCodeToStatus(curl_share_cleanup(share)));
}

std::unique_ptr<curl_slist, CurlSListDeleter> NewCurlSList(
    absl::Span<const std::string_view> strings) {
  std::unique_ptr<curl_slist, CurlSListDeleter> ret;
  for (std::string_view str : strings) {
    curl_slist *cur = curl_slist_append(ret.release(), str.data());
    CHECK(cur);
    ret.reset(cur);
  }
  return ret;
}

Status CurlEasyPerform(CURL *handle) {
  CURLcode code = curl_easy_perform(handle);
  if (code != CURLE_OK && code != CURLE_WRITE_ERROR) {
    return CurlCodeToStatus(code, handle);
  }

  long response_code = -1;
  RETURN_IF_ERROR(CurlEasyGetInfo(handle, CURLINFO_RESPONSE_CODE,
                                  &response_code));
  Status http_status = HTTPCodeToStatus(response_code);

  Status write_status;
  if (code == CURLE_WRITE_ERROR) {
    write_status = GetPrivate(handle)->last_write_error;
    CHECK(!write_status.ok());
  }

  if (http_status.ok()) {
    return write_status;
  }
  return StatusBuilder(std::move(http_status)) << write_status;
}

ThreadSafeCurlShare::ThreadSafeCurlShare()
  : share_(curl_share_init()), mutexes_(CreateMutexesMap()),
    locks_(CreateLocksMap()) {
  CHECK(share_);
  CHECK_OK(CurlShareSetopt(share_.get(), CURLSHOPT_USERDATA, this));
}

CURLSH *ThreadSafeCurlShare::ptr() const { return share_.get(); }

void ThreadSafeCurlShare::Lock(CURL *handle, curl_lock_data data,
                               curl_lock_access access, void *userptr) {
  reinterpret_cast<ThreadSafeCurlShare*>(userptr)->Lock(handle, data, access);
}

ThreadSafeCurlShare::MutexLock::~MutexLock() = default;

class ThreadSafeCurlShare::SharedMutexLock
  : public ThreadSafeCurlShare::MutexLock {
 public:
  SharedMutexLock(absl::Mutex *mu) : lock_(mu) {}
  ~SharedMutexLock() override = default;

 private:
  absl::ReaderMutexLock lock_;
};

class ThreadSafeCurlShare::SingleMutexLock
  : public ThreadSafeCurlShare::MutexLock {
 public:
  SingleMutexLock(absl::Mutex *mu) : lock_(mu) {}
  ~SingleMutexLock() override = default;

 private:
  absl::MutexLock lock_;
};

void ThreadSafeCurlShare::Lock(CURL*, curl_lock_data data,
                               curl_lock_access access) {
  const std::unique_ptr<absl::Mutex> &mu = mutexes_.at(data);
  std::unique_ptr<MutexLock> &lock = locks_.at(data);
  switch (access) {
    case CURL_LOCK_ACCESS_SHARED:
      lock.reset(new SharedMutexLock(mu.get()));
      break;
    case CURL_LOCK_ACCESS_SINGLE:
      lock.reset(new SingleMutexLock(mu.get()));
      break;
    default:
      CHECK(false);
  }
}

void ThreadSafeCurlShare::Unlock(CURL *handle, curl_lock_data data,
                                 void *userptr) {
  reinterpret_cast<ThreadSafeCurlShare*>(userptr)->Unlock(handle, data);
}

void ThreadSafeCurlShare::Unlock(CURL *handle, curl_lock_data data) {
  locks_.at(data).reset(nullptr);
}

Status CurlEasySetWriteCallback(
    CURL *handle, std::function<Status(std::string_view, size_t*)> user_cb) {
  decltype(&CurlWriteCallback) real_cb = nullptr;
  void *data = stdout;
  if (user_cb) {
    real_cb = &CurlWriteCallback;
    auto *priv = GetPrivate(handle);
    priv->write_callback = std::move(user_cb);
    data = priv;
  }
  RETURN_IF_ERROR(CurlEasySetopt(handle, CURLOPT_WRITEFUNCTION, real_cb));
  RETURN_IF_ERROR(CurlEasySetopt(handle, CURLOPT_WRITEDATA, data));
  return OkStatus();
}

Status CurlURLSet(CURLU *url, CURLUPart part, std::string_view content,
                  unsigned int flags) {
  CURLUcode code = curl_url_set(url, part, content.data(), flags);
  return CurlURLCodeToStatus(code);
}

StatusOr<std::unique_ptr<char, CurlStrDeleter>> CurlURLGet(
    CURLU *url, CURLUPart part, unsigned int flags) {
  char *val = nullptr;
  CURLUcode code = curl_url_get(url, part, &val, flags);
  RETURN_IF_ERROR(CurlURLCodeToStatus(code));
  return std::unique_ptr<char, CurlStrDeleter>(val);
}

std::unique_ptr<CURL, CurlHandleDeleter> CurlEasyInit() {
  std::unique_ptr<CURL, CurlHandleDeleter> handle(curl_easy_init());
  CHECK(handle);
  auto *priv = new CurlHandlePrivate();
  SetPrivate(handle.get(), priv);

  CHECK(curl_easy_setopt(handle.get(), CURLOPT_ERRORBUFFER,
                         &priv->error_buffer) == CURLE_OK);

  return handle;
}

Status CurlCodeToStatus(CURLcode code) {
  if (code == CURLE_OK) return OkStatus();
  const char *msg = curl_easy_strerror(code);
  StatusCode sc = StatusCode::kUnknown;
  switch (code) {
    case CURLE_FAILED_INIT:
      sc = StatusCode::kInternal;
      break;
    default:
      break;
  }
  return {sc, msg};
}

Status CurlCodeToStatus(CURLcode code, CURL *handle) {
  return StatusBuilder(CurlCodeToStatus(code))
      << GetPrivate(handle)->error_buffer;
}

Status CurlURLCodeToStatus(CURLUcode code) {
  if (code == CURLUE_OK) return OkStatus();
  std::string_view msg = "unknown URL error";
  StatusCode sc = StatusCode::kUnknown;
  // There is no curl_url_strerror, so we have to do this ourselves.
  // https://curl.haxx.se/mail/lib-2018-10/0112.html
  // Error messages are taken from
  // https://curl.haxx.se/libcurl/c/libcurl-errors.html
  switch (code) {
    case CURLUE_BAD_HANDLE:
      msg = "An argument that should be a CURLU pointer was passed in as a NULL.";
      sc = StatusCode::kInternal;
      break;
    case CURLUE_BAD_PARTPOINTER:
      msg = "A NULL pointer was passed to the 'part' argument of curl_url_get.";
      sc = StatusCode::kInternal;
      break;
    // there are more, but I got lazy
    default:
      break;
  }
  return {sc, msg};
}

Status CurlShareCodeToStatus(CURLSHcode code) {
  if (code == CURLSHE_OK) return OkStatus();
  const char *msg = curl_share_strerror(code);
  StatusCode sc = StatusCode::kUnknown;
  switch (code) {
    case CURLSHE_BAD_OPTION:
      sc = StatusCode::kInvalidArgument;
      break;
    case CURLSHE_IN_USE:
      sc = StatusCode::kUnavailable;
      break;
    case CURLSHE_INVALID:
      sc = StatusCode::kInvalidArgument;
      break;
    case CURLSHE_NOMEM:
      sc = StatusCode::kInternal;
      break;
    case CURLSHE_NOT_BUILT_IN:
      sc = StatusCode::kUnimplemented;
      break;
    default:
      break;
  }
  return {sc, msg};
}

Status HTTPCodeToStatus(int http_code) {
  auto code = StatusCode::kUnknown;
  if (http_code >= 200 && http_code < 300) {
    code = StatusCode::kOk;
  } else if (http_code >= 300 && http_code < 400) {
    code = StatusCode::kInvalidArgument;
  } else if (http_code >= 400 && http_code < 500) {
    switch (http_code) {
      case 400:
        return InvalidArgumentError("Bad request");
      case 401:
        return UnauthenticatedError("Unauthorized");
      case 403:
        return PermissionDeniedError("Forbidden");
      case 404:
        return NotFoundError("Not found");
      case 409:
        return AbortedError("Conflict");
      case 416:
        return OutOfRangeError("Requested range not satisfiable");
      case 429:
        return ResourceExhaustedError("Too many requests");
      case 499:
        return CancelledError("Client closed request");
      default:
        code = StatusCode::kFailedPrecondition;
    }
  } else if (http_code >= 500 && http_code < 600) {
    switch (http_code) {
      case 501:
        return UnimplementedError("Not implemented");
      case 503:
        return UnavailableError("Service unavailable");
      case 504:
        return DeadlineExceededError("Gateway Time-out");
      default:
        code = StatusCode::kInternal;
    }
  }
  return StatusBuilder({code, ""}) << "HTTP code " << http_code;
}

Status CurlGlobalInit() {
  static auto *status =
      new Status(CurlCodeToStatus(curl_global_init(CURL_GLOBAL_ALL)));
  return *status;
}

}  // namespace rhutil
