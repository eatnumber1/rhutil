load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

def rhutil_deps():
  if not native.existing_rule("bazel_skylib"):
    http_archive(
        name = "bazel_skylib",
        type = "tar.gz",
        url = "https://github.com/bazelbuild/bazel-skylib/releases/download/0.8.0/bazel-skylib.0.8.0.tar.gz",
        sha256 = "2ef429f5d7ce7111263289644d233707dba35e39696377ebab8b0bc701f7818e",
    )

  if not native.existing_rule("com_google_protobuf"):
    http_archive(
        name = "com_google_protobuf",
        sha256 = "b4fdd8e3733cd88dbe71ebbf553d16e536ff0d5eb1fdba689b8fc7821f65878a",
        strip_prefix = "protobuf-3.9.1",
        urls = ["https://github.com/protocolbuffers/protobuf/releases/download/v3.9.1/protobuf-cpp-3.9.1.zip"],
    )

  if not native.existing_rule("googletest"):
    git_repository(
        name = "googletest",
        remote = "https://github.com/google/googletest.git",
        commit = "90a443f9c2437ca8a682a1ac625eba64e1d74a8a",
        shallow_since = "1565193450 -0400",
        repo_mapping = {"@com_google_absl": "@abseil"}
    )

  if not native.existing_rule("abseil"):
    http_archive(
        name = "abseil",
        sha256 = "0b62fc2d00c2b2bc3761a892a17ac3b8af3578bd28535d90b4c914b0a7460d4e",
        strip_prefix = "abseil-cpp-20190808",
        urls = ["https://github.com/abseil/abseil-cpp/archive/20190808.zip"],
    )

  if not native.existing_rule("zlib"):
    http_archive(
        name = "zlib",
        sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
        strip_prefix = "zlib-1.2.11",
        build_file = "@//third_party:zlib.BUILD",
        urls = ["https://zlib.net/zlib-1.2.11.tar.gz"],
    )

  if not native.existing_rule("boringssl"):
    git_repository(
        name = "boringssl",
        commit = "7f9017dd3c60047d6fbc0f617d757c763af8867e",
        remote = "https://boringssl.googlesource.com/boringssl",
        shallow_since = "1542843106 +0000",
    )

  if not native.existing_rule("com_github_nelhage_rules_boost"):
    git_repository(
        name = "com_github_nelhage_rules_boost",
        commit = "13a7a40214bf62b6c5e44aa376db18c6315e67b2",
        remote = "https://github.com/nelhage/rules_boost",
        shallow_since = "1572449693 -0700",
    )

  if not native.existing_rule("platforms"):
    git_repository(
        name = "platforms",
        remote = "https://github.com/bazelbuild/platforms.git",
        commit = "46993efdd33b73649796c5fc5c9efb193ae19d51",
        shallow_since = "1573219050 -0800",
    )

  if not native.existing_rule("rules_foreign_cc"):
    git_repository(
        name = "rules_foreign_cc",
        remote = "https://github.com/bazelbuild/rules_foreign_cc.git",
        commit = "6bb0536452eaca3bad20c21ba6e7968d2eda004d",
        shallow_since = "1571839594 +0200",
    )
