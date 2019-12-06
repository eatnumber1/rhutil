load("@rules_foreign_cc//tools/build_defs:cmake.bzl", "cmake_external")

filegroup(
    name = "all",
    srcs = glob(["**"]),
)

config_setting(
    name = "macos",
    constraint_values = [
        "@platforms//os:macos",
    ],
)

cmake_external(
    name = "yajl",
    lib_source = ":all",
    visibility = ["//visibility:public"],
    static_libraries = ["libyajl_s.a"],
    shared_libraries = select({
        ":macos": ["libyajl.2.dylib"],
        "//conditions:default": ["libyajl.so"],
    }),
)
