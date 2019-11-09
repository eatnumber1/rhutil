#include "rhutil/json/json.h"

#include <string>

#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"

namespace rhutil {

using json = ::nlohmann::json;
using json_pointer = ::nlohmann::json_pointer<json>;
using json_sax = ::nlohmann::detail::json_sax_dom_callback_parser<json>;
using ParseEvent = json_sax::parse_event_t;
using Callback = ::rhutil::JSONParser::Callback;
using CallbackAction = ::rhutil::JSONParser::CallbackAction;

namespace {

StatusOr<CallbackAction> NopCallback(int, ParseEvent, json *) {
  return CallbackAction::KEEP;
}

Callback FixCallback(Callback callback) {
  if (!callback) return {&NopCallback};
  return callback;
}

}  // namespace

JSONParser::JSONParser() : JSONParser(&NopCallback) {}

JSONParser::JSONParser(Callback callback)
  : yajl_(this), callback_(FixCallback(std::move(callback))),
    sax_(root_,
         std::bind(&JSONParser::HandleSAXEvent, this, std::placeholders::_1,
                   std::placeholders::_2, std::placeholders::_3),
         /*allow_exceptions=*/false)
{}

Status JSONParser::Parse(std::string_view buf) {
  return yajl_.Parse(buf);
}

StatusOr<json> JSONParser::Complete(std::string_view last_buf) {
  RETURN_IF_ERROR(yajl_.Complete(last_buf));
  return std::move(root_);
}

Status JSONParser::Null() {
  CHECK(sax_.null());
  return std::move(last_error_);
}

Status JSONParser::Boolean(bool val) {
  CHECK(sax_.boolean(val));
  return std::move(last_error_);
}

Status JSONParser::Integer(int64_t val) {
  static_assert(std::is_same_v<json_sax::number_integer_t, int64_t>);
  CHECK(sax_.number_integer(val));
  return std::move(last_error_);
}

Status JSONParser::Double(double val) {
  static_assert(std::is_same_v<json_sax::number_float_t, double>);
  CHECK(sax_.number_float(val, /*unused=*/""));
  return std::move(last_error_);
}

Status JSONParser::String(std::string_view val) {
  std::string vs(val);
  CHECK(sax_.string(vs));
  return std::move(last_error_);
}

Status JSONParser::StartMap() {
  constexpr int kUnknownElementCount = -1;
  CHECK(sax_.start_object(kUnknownElementCount));
  return std::move(last_error_);
}

Status JSONParser::MapKey(std::string_view key) {
  std::string ks(key);
  CHECK(sax_.key(ks));
  return std::move(last_error_);
}

Status JSONParser::EndMap() {
  CHECK(sax_.end_object());
  return std::move(last_error_);
}

Status JSONParser::StartArray() {
  constexpr int kUnknownElementCount = -1;
  CHECK(sax_.start_array(kUnknownElementCount));
  return std::move(last_error_);
}

Status JSONParser::EndArray() {
  CHECK(sax_.end_array());
  return std::move(last_error_);
}

bool JSONParser::HandleSAXEvent(int depth, ParseEvent event, json &parsed) {
  auto action_or = callback_(depth, event, &parsed);
  if (!action_or.ok()) {
    last_error_ = std::move(action_or).status();
    return true;
  }
  last_error_ = OkStatus();

  switch (action_or.ValueOrDie()) {
    case CallbackAction::KEEP:
      return true;
    case CallbackAction::DISCARD:
      return false;
  }
}

}  // namespace rhutil
