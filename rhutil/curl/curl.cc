#include "rhutil/curl/curl.h"

#include <cstddef>
#include <cstdio>
#include <string>
#include <memory>
#include <string_view>

#include "rhutil/errno.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/numbers.h"

namespace rhutil {
namespace {

void CheckNullTerm(std::string_view sv) {
  CHECK(sv.data()[sv.size()] == '\0');
}

CURLU *CurlURLDupUnlessNull(CURLU *url) {
  if (url == nullptr) return nullptr;
  CURLU *ret = curl_url_dup(url);
  CHECK(ret);
  return ret;
}

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

void CurlStrDeleter::operator()(char *ptr) {
  curl_free(ptr);
}

void CurlShareDeleter::operator()(CURLSH *share) {
  CHECK_OK(CurlShareCodeToStatus(curl_share_cleanup(share)));
}

CurlURL::CurlURL(CURLU *url) : url_(url) {}

CurlURL::CurlURL() : CurlURL(curl_url()) {}
CurlURL::CurlURL(const CurlURL &u) : CurlURL(CurlURLDupUnlessNull(u.url_)) {}
CurlURL::CurlURL(CurlURL &&u) : CurlURL(nullptr) { swap(*this, u); }

CurlURL &CurlURL::operator=(CurlURL u) {
  swap(*this, u);
  return *this;
}

CurlURL::~CurlURL() {
  if (url_ == nullptr) return;
  curl_url_cleanup(url_);
}

void swap(CurlURL &a, CurlURL &b) {
  using std::swap;
  swap(a.url_, b.url_);
}

StatusOr<CurlURL> CurlURL::FromString(std::string_view url) {
  CurlURL ret;
  RETURN_IF_ERROR(ret.SetURL(url));
  return ret;
}

CurlURL CurlURL::FromStringOrDie(std::string_view url) {
  auto url_or = CurlURL::FromString(url);
  CHECK_OK(url_or.status());
  return std::move(url_or).ValueOrDie();
}

CURLU *CurlURL::ReleaseCURLU() {
  auto *url = url_;
  url_ = nullptr;
  return url;
}

CURLU *CurlURL::GetCURLU() const { return url_; }

Status CurlURL::SetURL(std::string_view url) {
  CheckNullTerm(url);
  return Set(CURLUPART_URL, url.data(), CURLU_URLENCODE);
}
void CurlURL::SetUser(std::string_view user) {
  CheckNullTerm(user);
  CHECK_OK(Set(CURLUPART_USER, user.data(), CURLU_URLENCODE));
}
void CurlURL::SetPassword(std::string_view password) {
  CheckNullTerm(password);
  CHECK_OK(Set(CURLUPART_PASSWORD, password.data(), CURLU_URLENCODE));
}
void CurlURL::SetHost(std::string_view host) {
  CheckNullTerm(host);
  CHECK_OK(Set(CURLUPART_HOST, host.data(), CURLU_URLENCODE));
}
void CurlURL::SetPort(uint16_t port) {
  CHECK_OK(Set(CURLUPART_PORT, absl::StrCat(port).c_str(), CURLU_URLENCODE));
}
void CurlURL::SetPath(std::string_view path) {
  CheckNullTerm(path);
  CHECK_OK(Set(CURLUPART_PATH, path.data(), CURLU_URLENCODE));
}
void CurlURL::SetScheme(std::string_view scheme) {
  CheckNullTerm(scheme);
  CHECK_OK(Set(CURLUPART_SCHEME, scheme.data(), CURLU_URLENCODE));
}

std::unique_ptr<char, CurlStrDeleter> CurlURL::GetURL() const {
  return Get(CURLUPART_URL).ValueOrDie();
}
std::unique_ptr<char, CurlStrDeleter> CurlURL::GetScheme() const {
  return Get(CURLUPART_SCHEME, CURLU_DEFAULT_SCHEME).ValueOrDie();
}
std::unique_ptr<char, CurlStrDeleter> CurlURL::GetUser() const {
  return GetAndMapErrorToNull(CURLUPART_USER, CURLUE_NO_USER,
                              CURLU_URLDECODE).ValueOrDie();
}
std::unique_ptr<char, CurlStrDeleter> CurlURL::GetPassword() const {
  return GetAndMapErrorToNull(CURLUPART_PASSWORD, CURLUE_NO_PASSWORD,
                              CURLU_URLDECODE).ValueOrDie();
}
std::unique_ptr<char, CurlStrDeleter> CurlURL::GetHost() const {
  return GetAndMapErrorToNull(CURLUPART_HOST, CURLUE_NO_HOST,
                              CURLU_URLDECODE).ValueOrDie();
}
uint16_t CurlURL::GetPort() const {
  auto str = Get(CURLUPART_PORT, CURLU_DEFAULT_PORT).ValueOrDie();
  uint32_t ret;  // SimpleAtoi doesn't work with 16-bit numbers.
  CHECK(absl::SimpleAtoi(str.get(), &ret));
  return ret;
}
std::unique_ptr<char, CurlStrDeleter> CurlURL::GetPath() const {
  return Get(CURLUPART_PATH, CURLU_URLDECODE).ValueOrDie();
}

Status CurlURL::Set(CURLUPart part, std::string_view content,
                    unsigned int flags) {
  CURLUcode code = curl_url_set(url_, part, content.data(), flags);
  return StatusBuilder(CodeToStatus(code))
      << "Failed to set URL to '" << content << "'";
}

StatusOr<std::unique_ptr<char, CurlStrDeleter>> CurlURL::GetAndMapErrorToNull(
    CURLUPart part, CURLUcode nullcode, unsigned int flags) const {
  if (nullcode == 0) return InvalidArgumentError("nullcode must not be 0");
  char *val = nullptr;
  CURLUcode code = curl_url_get(url_, part, &val, flags);
  if (code == nullcode) return {nullptr};
  RETURN_IF_ERROR(CodeToStatus(code));
  return std::unique_ptr<char, CurlStrDeleter>(val);
}

StatusOr<std::unique_ptr<char, CurlStrDeleter>> CurlURL::Get(
    CURLUPart part, unsigned int flags) const {
  char *val = nullptr;
  CURLUcode code = curl_url_get(url_, part, &val, flags);
  RETURN_IF_ERROR(CodeToStatus(code));
  return std::unique_ptr<char, CurlStrDeleter>(val);
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
    case CURLE_COULDNT_CONNECT:
      sc = StatusCode::kFailedPrecondition;
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

Status CurlURL::CodeToStatus(CURLUcode code) {
  if (code == CURLUE_OK) return OkStatus();
  std::string msg;
  StatusCode sc = StatusCode::kUnknown;
  // There is no curl_url_strerror, so we have to do this ourselves.
  // https://curl.haxx.se/mail/lib-2018-10/0112.html
  // Error messages are taken from
  // https://curl.haxx.se/libcurl/c/libcurl-errors.html
  switch (code) {
    case CURLUE_BAD_HANDLE:
      msg = "An argument that should be a CURLU pointer was passed in as a NULL";
      sc = StatusCode::kInternal;
      break;
    case CURLUE_BAD_PARTPOINTER:
      msg = "A NULL pointer was passed to the 'part' argument of curl_url_get";
      sc = StatusCode::kInternal;
      break;
    case CURLUE_NO_USER:
      msg = "There is no user part in the URL";
      sc = StatusCode::kFailedPrecondition;
      break;
    case CURLUE_MALFORMED_INPUT:
      msg = "A malformed input was passed to a URL API function.";
      sc = StatusCode::kInvalidArgument;
      break;
    case CURLUE_NO_HOST:
      msg = "There is no host part in the URL.";
      sc = StatusCode::kFailedPrecondition;
    // there are more, but I got lazy
    default:
      msg = absl::StrCat("unknown URL error ", code);
      break;
  }
  return {sc, std::move(msg)};
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

bool AbslParseFlag(absl::string_view text, CurlURL *url, std::string *error) {
  auto st = url->SetURL(text);
  if (!st.ok()) {
    *error = st.ToString();
    return false;
  }
  return true;
}

std::string AbslUnparseFlag(CurlURL url) {
  auto url_str_or = url.Get(CURLUPART_URL);
  if (!url_str_or.ok()) return "";
  return {std::move(url_str_or).ValueOrDie().get()};
}

}  // namespace rhutil
