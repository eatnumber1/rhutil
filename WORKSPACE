workspace(name = "rhutil")

load("@//:deps.bzl", "rhutil_deps")
rhutil_deps()
load("@//rhutil/curl:deps.bzl", "rhutil_curl_deps")
rhutil_curl_deps()
load("@//rhutil/json:deps.bzl", "rhutil_json_deps")
rhutil_json_deps()

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")
protobuf_deps()
load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
bazel_skylib_workspace()
load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()

load("@rules_foreign_cc//:workspace_definitions.bzl", "rules_foreign_cc_dependencies")
rules_foreign_cc_dependencies(
#    native_tools_toolchains = [
#        "@cdmanip//tools/build_defs:eatnumber1_cmake_toolchain_osx",
#        "@cdmanip//tools/build_defs:eatnumber1_ninja_toolchain_osx",
#    ],
#    register_default_tools = False
)
