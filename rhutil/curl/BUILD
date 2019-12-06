package(default_visibility = ["//rhutil:internal"])

cc_library(
    name = "curl",
    hdrs = ["curl.h"],
    srcs = ["curl.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//rhutil:status",
        "//rhutil:errno",
        "@abseil//absl/types:span",
        "@abseil//absl/synchronization",
        "@abseil//absl/container:flat_hash_map",
        "@abseil//absl/strings",
        "@curl//:curl",
    ],
)
