#ifndef RHUTIL_JSON_YAJL_H_
#define RHUTIL_JSON_YAJL_H_

#include <string_view>
#include <cstdint>
#include <cstddef>

#include "rhutil/status.h"
#include "yajl/yajl_parse.h"

namespace rhutil {

class YAJLParser {
 public:
  class Callbacks {
   public:
    virtual ~Callbacks() = 0;
    virtual Status Null() = 0;
    virtual Status Boolean(bool val) = 0;
    virtual Status Integer(int64_t val) = 0;
    virtual Status Double(double val) = 0;
    virtual Status String(std::string_view val) = 0;
    virtual Status StartMap() = 0;
    virtual Status MapKey(std::string_view key) = 0;
    virtual Status EndMap() = 0;
    virtual Status StartArray() = 0;
    virtual Status EndArray() = 0;
  };

  class Allocator {
   public:
    virtual ~Allocator() = 0;
    virtual void *Malloc(std::size_t sz) = 0;
    virtual void Free(void *ptr) = 0;
    virtual void *Realloc(void *ptr, std::size_t sz) = 0;
  };

  YAJLParser(Callbacks *callbacks, Allocator *allocator = nullptr);
  ~YAJLParser();

  // Because this class is used as the ctx pointer for yajl callbacks, moving
  // a YAJLParser (which would invalidate pointers) is impossible.
  YAJLParser(YAJLParser&&) = delete;
  YAJLParser &operator=(YAJLParser&&) = delete;
  // Because yajl_handle holds internal parse state, and no facilities for
  // copying it are provided, copying a YAJLParser is impossible.
  YAJLParser(const YAJLParser&) = delete;
  YAJLParser &operator=(const YAJLParser&) = delete;

  Status Parse(std::string_view buf);
  Status Complete(std::string_view last_parse_buf = {});

 private:
  Status CodeToStatus(yajl_status ystat, std::string_view buf = {});

  static yajl_callbacks *GetCallbacksTable();
  static int OnNull(void*);
  static int OnBoolean(void*, int);
  static int OnInteger(void*, long long);
  static int OnDouble(void*, double);
  static int OnString(void*, const unsigned char *, size_t);
  static int OnStartMap(void*);
  static int OnMapKey(void*, const unsigned char *, size_t);
  static int OnEndMap(void*);
  static int OnStartArray(void*);
  static int OnEndArray(void*);

  static yajl_alloc_funcs *GetAllocatorTable();
  static void *Malloc(void*, unsigned long);
  static void Free(void*, void*);
  static void *Realloc(void*, void*, unsigned long);

  yajl_handle_t *handle_;
  Callbacks *callbacks_;
  Allocator *allocator_;
  Status last_error_;
};

}  // namespace rhutil

#endif  // RHUTIL_JSON_YAJL_H_
