#ifndef RHUTIL_JSON_JSON_H_
#define RHUTIL_JSON_JSON_H_

#include <string_view>
#include <memory>
#include <cstdint>
#include <functional>

#include "rhutil/status.h"
#include "nlohmann/json.hpp"
#include "rhutil/json/yajl.h"

namespace rhutil {

class JSONParser : private YAJLParser::Callbacks {
 public:
  enum class CallbackAction {
    KEEP, DISCARD
  };
  using ParseEvent = ::nlohmann::json::parse_event_t;
  using Callback =
    std::function<StatusOr<CallbackAction>(int depth, ParseEvent event,
                                           nlohmann::json *parsed)>;

  JSONParser();
  explicit JSONParser(Callback callback);

  JSONParser(const JSONParser &) = delete;
  JSONParser &operator=(const JSONParser &) = delete;
  JSONParser(JSONParser &&) = delete;
  JSONParser &operator=(JSONParser &&) = delete;

  Status Parse(std::string_view buf);
  StatusOr<nlohmann::json> Complete(std::string_view last_buf = {});

 private:
  Status Null() override;
  Status Boolean(bool val) override;
  Status Integer(int64_t val) override;
  Status Double(double val) override;
  Status String(std::string_view val) override;
  Status StartMap() override;
  Status MapKey(std::string_view key) override;
  Status EndMap() override;
  Status StartArray() override;
  Status EndArray() override;

  using json_sax =
      ::nlohmann::detail::json_sax_dom_callback_parser<nlohmann::json>;

  bool HandleSAXEvent(int depth, json_sax::parse_event_t event,
                      nlohmann::json &parsed);

  YAJLParser yajl_;
  Callback callback_;
  nlohmann::json root_;
  json_sax sax_;
  Status last_error_;
};

}  // namespace rhutil

#endif  // RHUTIL_JSON_JSON_H_
