workspace(name = "dbc_parser")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# MIGRATION NOTICE
# =============================================================================
# This project is in the process of migrating from WORKSPACE to Bzlmod/MODULE.bazel
# 
# Step 1: External dependencies have been moved to MODULE.bazel
# Step 2: Next steps will include:
#   - Updating all BUILD files to use the modern repository references
#   - Completing the transition to Bzlmod-only dependency management
#
# For BUILD files modification:
# @com_google_googletest//:gtest_main  -->  @googletest//:gtest_main
# @com_github_taocpp_pegtl//:pegtl    -->  @taocpp_pegtl//:pegtl
# =============================================================================

# GoogleTest
http_archive(
    name = "com_google_googletest",
    urls = ["https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip"],
    strip_prefix = "googletest-1.14.0",
    sha256 = "1f357c27ca988c3f7c6b4bf68a9395005ac6761f034046e9dde0896e3aba00e4",
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
    hdrs = glob(["include/tao/pegtl/**/*.hpp", "include/tao/pegtl.hpp"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
""",
)

# Alias for googletest with the new repository name
http_archive(
    name = "googletest",
    urls = ["https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip"],
    strip_prefix = "googletest-1.14.0",
    sha256 = "1f357c27ca988c3f7c6b4bf68a9395005ac6761f034046e9dde0896e3aba00e4",
)

# Alias for taocpp_pegtl with consistent naming
http_archive(
    name = "taocpp_pegtl",
    urls = ["https://github.com/taocpp/PEGTL/archive/3.2.7.zip"],
    strip_prefix = "PEGTL-3.2.7",
    sha256 = "f2ef563c0b0c3488b3aadb5404a8ce7a78985b4b0a9e0a96702305478c6e5af7",
    build_file_content = """
cc_library(
    name = "pegtl",
    hdrs = glob(["include/tao/pegtl/**/*.hpp", "include/tao/pegtl.hpp"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
""",
) 