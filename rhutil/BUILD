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
    ],
)

cc_test(
    name = "status_test",
    testonly = 1,
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
