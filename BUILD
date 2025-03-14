load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

cc_library(
    name = "dbc_parser",
    srcs = glob([
        "src/dbc_parser/**/*.cpp",
    ]),
    hdrs = glob([
        "include/dbc_parser/**/*.h",
    ]),
    includes = ["include"],
    deps = [
        "@boost.spirit//:boost.spirit",
        "@boost.fusion//:boost.fusion",
        "@boost.phoenix//:boost.phoenix",
        "@boost.variant//:boost.variant",
        "@boost.optional//:boost.optional",
        "@pegtl",
    ],
    copts = [
        "-std=c++17",
        "-DBOOST_PHOENIX_STL_TUPLE_H_",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "dbc_parser_cli",
    srcs = ["tools/dbc_parser_cli.cpp"],
    deps = [
        ":dbc_parser",
        "@boost.program_options//:boost.program_options",
    ],
    copts = [
        "-std=c++17",
        "-DBOOST_PHOENIX_STL_TUPLE_H_",
    ],
)

# Create a run_tests.sh script for easy running of all tests
sh_binary(
    name = "run_tests",
    srcs = ["run_tests.sh"],
)