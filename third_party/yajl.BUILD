load("@rules_foreign_cc//tools/build_defs:cmake.bzl", "cmake_external")

filegroup(
    name = "all",
    srcs = glob(["**"]),
)

cmake_external(
    name = "yajl",
    lib_source = ":all",
    visibility = ["//visibility:public"],
    static_libraries = ["libyajl_s.a"],
    shared_libraries = select({
        "@bazel_tools//src/conditions:windows": ["libyajl.2.dll"],
        "@bazel_tools//src/conditions:darwin": ["libyajl.2.dylib"],
        "//conditions:default": ["libyajl.2.so"],
    }),
)
