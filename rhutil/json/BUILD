package(default_visibility = ["//rhutil:internal"])

cc_library(
    name = "json",
    hdrs = ["json.h"],
    srcs = ["json.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":yajl",
        "//rhutil:status",
        "@nlohmann_json//:json",
        "@abseil//absl/strings",
    ],
)

cc_library(
    name = "yajl",
    hdrs = ["yajl.h"],
    srcs = ["yajl.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//rhutil:status",
        "@yajl//:yajl",
    ],
)
