package(default_visibility = [":internal"])

package_group(
    name = "internal",
    packages = ["//rhutil/..."],
)

cc_library(
    name = "cleanup",
    visibility = ["//visibility:public"],
    srcs = ["cleanup.cc"],
    hdrs = ["cleanup.h"],
)

cc_library(
    name = "status",
    visibility = ["//visibility:public"],
    srcs = ["status.cc"],
    hdrs = ["status.h"],
    deps = [
        "@abseil//absl/strings:str_format",
        "@abseil//absl/strings",
        "@abseil//absl/base:core_headers",
    ],
)

cc_test(
    name = "status_test",
    srcs = ["status_test.cc"],
    deps = [
        ":status",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "errno",
    visibility = ["//visibility:public"],
    srcs = ["errno.cc"],
    hdrs = ["errno.h"],
    deps = [
        ":status",
    ],
)

cc_library(
    name = "file",
    visibility = ["//visibility:public"],
    srcs = ["file.cc"],
    hdrs = ["file.h"],
    deps = [
        ":status",
        ":errno",
        "@abseil//absl/strings",
        "@abseil//absl/strings:str_format",
    ],
)
