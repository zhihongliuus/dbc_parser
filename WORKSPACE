workspace(name = "dbc_parser_workspace")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Relying entirely on Bazel's default C++ toolchain detection.
# Explicit rules_cc and cc_configure removed.

http_archive(
    name = "pegtl",
    urls = ["https://github.com/taocpp/PEGTL/archive/refs/tags/3.2.7.tar.gz"],
    sha256 = "d6cd113d8bd14e98bcbe7b7f8fc1e1e33448dc359e8cd4cca30e034ec2f0642d",
    strip_prefix = "PEGTL-3.2.7",
    build_file_content = """
cc_library(
    name = "pegtl",
    hdrs = glob(["include/**/*.hpp"]),
    strip_include_prefix = "include",
    include_prefix = ".",
    visibility = ["//visibility:public"],
)
""",
)

# Optional: Google Test
# http_archive(
#     name = "com_google_googletest",
#     sha256 = "...", 
#     strip_prefix = "googletest-release-VERSION",
#     urls = ["https://github.com/google/googletest/archive/refs/tags/release-VERSION.tar.gz"],
# ) 