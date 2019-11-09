load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def rhutil_json_deps():
  if not native.existing_rule("yajl"):
    http_archive(
        name = "yajl",
        strip_prefix = "lloyd-yajl-66cb08c",
        sha256 = "510a13e0be57cd4ba99e60ac806a3635854af51316d3131d3742a90298ccde38",
        build_file = "@rhutil//third_party:yajl.BUILD",
        urls = ["http://github.com/lloyd/yajl/tarball/2.1.0"],
        type = "tar.gz",
    )

  if not native.existing_rule("nlohmann_json"):
    http_archive(
        name = "nlohmann_json",
        strip_prefix = "include",
        sha256 = "541c34438fd54182e9cdc68dd20c898d766713ad6d901fb2c6e28ff1f1e7c10d",
        build_file = "@rhutil//third_party:json.BUILD",
        urls = ["https://github.com/nlohmann/json/releases/download/v3.7.0/include.zip"],
    )
