cc_library(
    name = "comment_parser",
    srcs = ["comment_parser.cc"],
    hdrs = ["comment_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "@taocpp_pegtl//:pegtl",
    ],
)

cc_library(
    name = "comment",
    visibility = ["//visibility:public"],
    deps = [
        ":comment_parser",
    ],
) 