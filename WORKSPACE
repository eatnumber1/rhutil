workspace(name = "cdmanip")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "googletest",
    remote = "https://github.com/google/googletest.git",
    commit = "90a443f9c2437ca8a682a1ac625eba64e1d74a8a",
    shallow_since = "1565193450 -0400",
)

http_archive(
    name = "abseil",
    sha256 = "0b62fc2d00c2b2bc3761a892a17ac3b8af3578bd28535d90b4c914b0a7460d4e",
    strip_prefix = "abseil-cpp-20190808",
    urls = ["https://github.com/abseil/abseil-cpp/archive/20190808.zip"],
)
