load("@rules_cc//cc:defs.bzl", "cc_test")

filegroup(
    name = "sample_files",
    srcs = glob(["samples/*"]),
    visibility = ["//visibility:public"],
)

# Tests re-enabled
cc_test(
    name = "parser_test",
    srcs = ["parser_test.cpp"],
    deps = [
        "//:dbc_parser",
        "@googletest//:gtest_main",
    ],
    data = [
        "test.dbc",
    ],
    copts = [
        "-std=c++17",
        "-DBOOST_PHOENIX_STL_TUPLE_H_",
    ],
)

cc_test(
    name = "signal_decoder_test",
    srcs = ["signal_decoder_test.cpp"],
    deps = [
        "//:dbc_parser",
        "@googletest//:gtest_main",
    ],
    copts = [
        "-std=c++17",
        "-DBOOST_PHOENIX_STL_TUPLE_H_",
    ],
)

cc_test(
    name = "decoder_test",
    srcs = ["decoder_test.cpp"],
    deps = [
        "//:dbc_parser",
        "@googletest//:gtest_main",
    ],
    copts = [
        "-std=c++17",
        "-DBOOST_PHOENIX_STL_TUPLE_H_",
    ],
)

cc_test(
    name = "error_handling_test",
    srcs = ["error_handling_test.cpp"],
    deps = [
        "//:dbc_parser",
        "@googletest//:gtest_main",
    ],
    data = [":sample_files"],
    copts = [
        "-std=c++17",
        "-DBOOST_PHOENIX_STL_TUPLE_H_",
    ],
)

cc_test(
    name = "integration_test",
    srcs = ["integration_test.cpp"],
    deps = [
        "//:dbc_parser",
        "@googletest//:gtest_main",
    ],
    data = [":sample_files"],
    copts = [
        "-std=c++17",
        "-DBOOST_PHOENIX_STL_TUPLE_H_",
    ],
)

cc_test(
    name = "dbc_types_test",
    srcs = ["dbc_types_test.cpp"],
    deps = [
        "//:dbc_parser",
        "@googletest//:gtest_main",
    ],
    copts = [
        "-std=c++17",
        "-DBOOST_PHOENIX_STL_TUPLE_H_",
    ],
)

cc_test(
    name = "dbc_components_test",
    srcs = ["dbc_components_test.cpp"],
    deps = [
        "//:dbc_parser",
        "@googletest//:gtest_main",
    ],
    copts = [
        "-std=c++17",
        "-DBOOST_PHOENIX_STL_TUPLE_H_",
    ],
)
