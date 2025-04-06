cc_library(
    name = "dbc_file_parser",
    srcs = [
        "dbc_file_parser.cc",
    ],
    hdrs = [
        "dbc_file_parser.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "//src/dbc_parser/parser/attribute:attribute",
        "//src/dbc_parser/parser/base:base",
        "//src/dbc_parser/parser/comment:comment",
        "//src/dbc_parser/parser/environment:environment",
        "//src/dbc_parser/parser/message:message",
        "//src/dbc_parser/parser/value:value",
        "@taocpp_pegtl//:pegtl",
    ],
)

cc_library(
    name = "parser",
    visibility = ["//visibility:public"],
    deps = [
        ":dbc_file_parser",
    ],
) 