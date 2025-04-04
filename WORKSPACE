load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "taocpp_pegtl",
    urls = ["https://github.com/taocpp/PEGTL/archive/refs/tags/3.2.7.zip"],
    strip_prefix = "PEGTL-3.2.7",
    sha256 = "d5f69da51968a7d58a1be0e4d7a2f4bba8c3c713f12142c0345353c01c7f5d4c",
    build_file_content = """
cc_library(
    name = "pegtl",
    hdrs = glob(["include/tao/pegtl/**/*.hpp"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
""",
)

http_archive(
    name = "googletest",
    urls = ["https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip"],
    strip_prefix = "googletest-1.14.0",
    sha256 = "1f357c27ca988c3f7c6b4bf68a9395005ac6761f034046e9dde0896e3aba00e4",
) 