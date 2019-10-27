package(
    default_visibility = ["//rhutil:internal"],
    default_testonly = 1,
)

cc_library(
    name = "assertions",
    hdrs = ["assertions.h"],
    srcs = ["assertions.cc"],
    deps = [
        "//rhutil:status",
        "@googletest//:gtest"
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "protobuf_assertions",
    hdrs = ["protobuf_assertions.h"],
    srcs = ["protobuf_assertions.cc"],
    deps = [
        "@googletest//:gtest",
        "@com_google_protobuf//:protobuf",
    ],
    visibility = ["//visibility:public"],
)
