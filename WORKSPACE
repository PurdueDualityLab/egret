workspace(name = "com_github_dualitylab_egret")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# google test - testing framework
http_archive(
    name = "com_google_googletest",
    strip_prefix = "googletest-609281088cfefc76f9d0ce82e1ff6c30cc3591e5",
    urls = ["https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip"],
)

# fmtlib - used for formatting strings
http_archive(
    name = "com_github_fmtlib_fmt",
    build_file = "@//:third_party/fmtlib.BUILD",
    sha256 = "cceb4cb9366e18a5742128cb3524ce5f50e88b476f1e54737a47ffdf4df4c996",
    strip_prefix = "fmt-9.1.0",
    url = "https://github.com/fmtlib/fmt/releases/download/9.1.0/fmt-9.1.0.zip",
)

# github.com/nlohmann/json - C++ JSON parsing utilities
http_archive(
    name = "com_github_nlohmann_json",
    build_file = "@//:third_party/nlohmann_json.BUILD",
    sha256 = "d69f9deb6a75e2580465c6c4c5111b89c4dc2fa94e3a85fcd2ffcd9a143d9273",
    strip_prefix = "json-3.11.2",
    url = "https://github.com/nlohmann/json/archive/refs/tags/v3.11.2.tar.gz",
)
