load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "dbc_parser_lib",
    srcs = glob(["**/*.cc"]),
    hdrs = glob(["**/*.h"]),
    deps = ["@pegtl//:pegtl"],
    copts = [
        "-std=c++17",
    ],
    includes = ["."],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "dbc_parser",
    srcs = ["main.cc"],
    deps = [":dbc_parser_lib"],
)

# Optional: Add a target for the parser executable if needed directly
# cc_binary(
#     name = "dbc_parser_main",
#     srcs = ["main.cc"], # Example main file
#     deps = [":dbc_parser_lib"],
# ) 