load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "dbc_parser",
    srcs = ["//src:dbc_parser.cc"],
    hdrs = ["//src:dbc_parser.h"],
    deps = [
        "//src/dbc_types:dbc_types",
        "//src/grammar:grammar",
    ],
) 