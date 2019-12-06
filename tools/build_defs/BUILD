load("@rules_foreign_cc//tools/build_defs/native_tools:native_tools_toolchain.bzl", "native_tool_toolchain")

native_tool_toolchain(
    name = "eatnumber1_cmake",
    path = "/Users/eatnumber1/homebrew/bin/cmake",
    visibility = ["//visibility:public"],
)

toolchain(
  name = "eatnumber1_cmake_toolchain_osx",
  toolchain = ":eatnumber1_cmake",
  toolchain_type = "@rules_foreign_cc//tools/build_defs:cmake_toolchain",
  #exec_compatible_with = [
  #    "@bazel_tools//platforms:osx",
  #    "@bazel_tools//platforms:x86_64",
  #],
)

native_tool_toolchain(
    name = "eatnumber1_ninja",
    path = "/Users/eatnumber1/homebrew/bin/ninja",
    visibility = ["//visibility:public"],
)

toolchain(
  name = "eatnumber1_ninja_toolchain_osx",
  toolchain = ":eatnumber1_ninja",
  toolchain_type = "@rules_foreign_cc//tools/build_defs:ninja_toolchain",
  #exec_compatible_with = [
  #    "@bazel_tools//platforms:osx",
  #    "@bazel_tools//platforms:x86_64",
  #],
)
