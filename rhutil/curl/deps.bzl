load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def rhutil_curl_deps():
  if not native.existing_rule("curl"):
    http_archive(
        name = "curl",
        build_file = "@rhutil//third_party:curl.BUILD",
        sha256 = "d0393da38ac74ffac67313072d7fe75b1fa1010eb5987f63f349b024a36b7ffb",
        strip_prefix = "curl-7.66.0",
        urls = ["https://curl.haxx.se/download/curl-7.66.0.tar.gz"],
    )
