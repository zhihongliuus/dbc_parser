load("@rules_cc//cc:defs.bzl", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_test(
    name = "dbc_parser_test",
    srcs = ["dbc_parser_test.cc"],
    deps = [
        "//:dbc_parser",
        "@googletest//:gtest_main",
    ],
    data = [
        "//test:test_data",
    ],
) 