load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "dbc_parser_lib",
    srcs = ["dbc_parser.cc"],
    hdrs = ["dbc_parser.h"],
    deps = [
        "//src/dbc_types:dbc_types",
        "//src/grammar:grammar",
        # PEGTL dependency will be added back when we implement the grammar
        # "@com_github_taocpp_pegtl//:pegtl",
    ],
) 