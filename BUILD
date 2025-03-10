load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "dbc_parser",
    srcs = glob(
        [
            "src/**/*.cpp",
        ],
        # No exclusions after fixing Boost macro issues
    ),
    hdrs = glob(["include/**/*.h"]),
    copts = [
        "-std=c++17",
    ],
    includes = ["include"],
    visibility = ["//visibility:public"],
)

# Re-enable tests now that the Boost macro issues are fixed
cc_test(
    name = "dbc_parser_test",
    srcs = glob(
        [
            "test/**/*.cpp",
        ],
        allow_empty = True,
    ),
    deps = [
        ":dbc_parser",
        "@googletest//:gtest_main",
    ],
)

cc_binary(
    name = "dbc_parser_cli",
    srcs = ["tools/dbc_parser_cli.cpp"],
    deps = [
        ":dbc_parser",
    ],
) 