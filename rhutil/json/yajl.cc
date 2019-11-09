#include "rhutil/json/yajl.h"

#include <utility>
#include <type_traits>

namespace rhutil {

using Callbacks = ::rhutil::YAJLParser::Callbacks;

Status YAJLParser::CodeToStatus(yajl_status ystat, std::string_view buf) {
  auto status = UnknownError("");
  switch (ystat) {
    case yajl_status_ok:
      return OkStatus();
    case yajl_status_client_canceled:
      CHECK(!last_error_.ok());
      status = last_error_;
      break;
    case yajl_status_error:
      status = UnknownError("An error occured while parsing JSON");
      break;
  }

  unsigned char *errmsg = yajl_get_error(
      handle_, /*verbose=*/!buf.empty(),
      reinterpret_cast<const unsigned char *>(buf.data()), buf.size());
  status = StatusBuilder(status) << errmsg;
  yajl_free_error(handle_, errmsg);
  return status;
}

YAJLParser::YAJLParser(Callbacks *callbacks, Allocator *allocator)
  : callbacks_(callbacks),
    allocator_(allocator) {
  yajl_alloc_funcs *alloc_funcs = allocator ? GetAllocatorTable() : nullptr;
  handle_ = yajl_alloc(GetCallbacksTable(), alloc_funcs, this);
}

YAJLParser::~YAJLParser() {
  yajl_free(handle_);
}

Status YAJLParser::Parse(std::string_view buf) {
  yajl_status ystat = yajl_parse(
      handle_, reinterpret_cast<const unsigned char *>(buf.data()), buf.size());
  return CodeToStatus(ystat, buf);
}

Status YAJLParser::Complete(std::string_view last_parse_buf) {
  return CodeToStatus(yajl_complete_parse(handle_), last_parse_buf);
}

YAJLParser::Callbacks::~Callbacks() = default;

yajl_callbacks *YAJLParser::GetCallbacksTable() {
  static yajl_callbacks *callbacks = []() {
    auto *callbacks = new yajl_callbacks();
    callbacks->yajl_null = &YAJLParser::OnNull;
    callbacks->yajl_boolean = &YAJLParser::OnBoolean;
    callbacks->yajl_integer = &YAJLParser::OnInteger;
    callbacks->yajl_double = &YAJLParser::OnDouble;
    callbacks->yajl_number = nullptr;
    callbacks->yajl_string = &YAJLParser::OnString;
    callbacks->yajl_start_map = &YAJLParser::OnStartMap;
    callbacks->yajl_map_key = &YAJLParser::OnMapKey;
    callbacks->yajl_end_map = &YAJLParser::OnEndMap;
    callbacks->yajl_start_array = &YAJLParser::OnStartArray;
    callbacks->yajl_end_array = &YAJLParser::OnEndArray;
    return callbacks;
  }();
  return callbacks;
}
int YAJLParser::OnNull(void *ctx) {
  auto *parser = reinterpret_cast<YAJLParser*>(ctx);
  auto err = parser->callbacks_->Null();
  if (!err.ok()) parser->last_error_ = std::move(err);
  return err.ok();
}
int YAJLParser::OnBoolean(void *ctx, int boolVal) {
  auto *parser = reinterpret_cast<YAJLParser*>(ctx);
  auto err = parser->callbacks_->Boolean(boolVal);
  if (!err.ok()) parser->last_error_ = std::move(err);
  return err.ok();
}
int YAJLParser::OnInteger(void *ctx, long long integerVal) {
  static_assert(std::is_same_v<long long, int64_t>);
  auto *parser = reinterpret_cast<YAJLParser*>(ctx);
  auto err = parser->callbacks_->Integer(integerVal);
  if (!err.ok()) parser->last_error_ = std::move(err);
  return err.ok();
}
int YAJLParser::OnDouble(void *ctx, double doubleVal) {
  auto *parser = reinterpret_cast<YAJLParser*>(ctx);
  auto err = parser->callbacks_->Double(doubleVal);
  if (!err.ok()) parser->last_error_ = std::move(err);
  return err.ok();
}
int YAJLParser::OnString(void *ctx, const unsigned char *stringVal,
                         size_t stringLen) {
  auto *parser = reinterpret_cast<YAJLParser*>(ctx);
  auto err = parser->callbacks_->String({
      reinterpret_cast<const char *>(stringVal), stringLen});
  if (!err.ok()) parser->last_error_ = std::move(err);
  return err.ok();
}
int YAJLParser::OnStartMap(void *ctx) {
  auto *parser = reinterpret_cast<YAJLParser*>(ctx);
  auto err = parser->callbacks_->StartMap();
  if (!err.ok()) parser->last_error_ = std::move(err);
  return err.ok();
}
int YAJLParser::OnMapKey(void *ctx, const unsigned char *stringVal,
                         size_t stringLen) {
  auto *parser = reinterpret_cast<YAJLParser*>(ctx);
  auto err = parser->callbacks_->MapKey({
      reinterpret_cast<const char *>(stringVal), stringLen});
  if (!err.ok()) parser->last_error_ = std::move(err);
  return err.ok();
}
int YAJLParser::OnEndMap(void *ctx) {
  auto *parser = reinterpret_cast<YAJLParser*>(ctx);
  auto err = parser->callbacks_->EndMap();
  if (!err.ok()) parser->last_error_ = std::move(err);
  return err.ok();
}
int YAJLParser::OnStartArray(void *ctx) {
  auto *parser = reinterpret_cast<YAJLParser*>(ctx);
  auto err = parser->callbacks_->StartArray();
  if (!err.ok()) parser->last_error_ = std::move(err);
  return err.ok();
}
int YAJLParser::OnEndArray(void *ctx) {
  auto *parser = reinterpret_cast<YAJLParser*>(ctx);
  auto err = parser->callbacks_->EndArray();
  if (!err.ok()) parser->last_error_ = std::move(err);
  return err.ok();
}

YAJLParser::Allocator::~Allocator() = default;

yajl_alloc_funcs *YAJLParser::GetAllocatorTable() {
  static yajl_alloc_funcs *allocator = []() {
    auto *allocator = new yajl_alloc_funcs();
    allocator->malloc = &YAJLParser::Malloc;
    allocator->free = &YAJLParser::Free;
    allocator->realloc = &YAJLParser::Realloc;
    return allocator;
  }();
  return allocator;
}
void *YAJLParser::Malloc(void *ctx, unsigned long sz) {
  static_assert(std::is_same_v<decltype(sz), std::size_t>);
  auto *parser = reinterpret_cast<YAJLParser*>(ctx);
  return parser->allocator_->Malloc(sz);
}
void YAJLParser::Free(void *ctx, void *ptr) {
  auto *parser = reinterpret_cast<YAJLParser*>(ctx);
  parser->allocator_->Free(ptr);
}
void *YAJLParser::Realloc(void *ctx, void *ptr, unsigned long sz) {
  static_assert(std::is_same_v<decltype(sz), std::size_t>);
  auto *parser = reinterpret_cast<YAJLParser*>(ctx);
  return parser->allocator_->Realloc(ptr, sz);
}


}  // namespace rhutil
