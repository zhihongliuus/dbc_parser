workspace(name = "dbc_parser")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# GoogleTest
http_archive(
    name = "com_google_googletest",
    urls = ["https://github.com/google/googletest/archive/release-1.11.0.zip"],
    strip_prefix = "googletest-release-1.11.0",
    sha256 = "353571c2440176ded91c2de6d6cd88ddd41401d14692ec1f99e35d013feda55a",
)

# PEGTL (Parsing Expression Grammar Template Library)
http_archive(
    name = "com_github_taocpp_pegtl",
    urls = ["https://github.com/taocpp/PEGTL/archive/3.2.7.zip"],
    strip_prefix = "PEGTL-3.2.7",
    sha256 = "f2ef563c0b0c3488b3aadb5404a8ce7a78985b4b0a9e0a96702305478c6e5af7",
    build_file_content = """
cc_library(
    name = "pegtl",
    hdrs = glob(["include/tao/pegtl/**/*.hpp"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
""",
) 