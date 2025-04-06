cc_library(
    name = "version_parser",
    srcs = ["version_parser.cc"],
    hdrs = ["version_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "@taocpp_pegtl//:pegtl",
    ],
)

cc_library(
    name = "new_symbols_parser",
    srcs = ["new_symbols_parser.cc"],
    hdrs = ["new_symbols_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "@taocpp_pegtl//:pegtl",
    ],
)

cc_library(
    name = "bit_timing_parser",
    srcs = ["bit_timing_parser.cc"],
    hdrs = ["bit_timing_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "@taocpp_pegtl//:pegtl",
    ],
)

cc_library(
    name = "nodes_parser",
    srcs = ["nodes_parser.cc"],
    hdrs = ["nodes_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "@taocpp_pegtl//:pegtl",
    ],
)

cc_library(
    name = "base",
    visibility = ["//visibility:public"],
    deps = [
        ":version_parser",
        ":new_symbols_parser",
        ":bit_timing_parser",
        ":nodes_parser",
    ],
) 