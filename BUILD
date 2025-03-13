load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "dbc_parser",
    srcs = glob(
        [
            "src/**/*.cpp",
        ],
        exclude = [
            "src/dbc_parser/dbc_grammar.cpp",
            "src/dbc_parser/dbc_parser.cpp",
        ],
    ),
    hdrs = glob(["include/**/*.h"]),
    copts = [
        "-std=c++17",
    ],
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        "@boost.spirit",
        "@boost.fusion",
        "@boost.variant",
        "@boost.optional",
        "@boost.phoenix",
        "@boost.filesystem",
        "@boost.program_options",
    ],
)

# Re-enable tests now that the Boost macro issues are fixed
cc_test(
    name = "dbc_parser_test",
    srcs = glob(
        [
            "test/**/*.cpp",
        ],
    ),
    deps = [
        ":dbc_parser",
        "@googletest//:gtest_main",
    ],
)

cc_binary(
    name = "dbc_parser_cli",
    srcs = ["tools/dbc_parser_cli.cpp"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        ":dbc_parser",
    ],
) 