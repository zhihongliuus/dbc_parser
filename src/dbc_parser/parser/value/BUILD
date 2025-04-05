cc_library(
    name = "value_table_parser",
    srcs = ["value_table_parser.cc"],
    hdrs = ["value_table_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
)

cc_library(
    name = "value_description_parser",
    srcs = ["value_description_parser.cc"],
    hdrs = ["value_description_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
)

cc_library(
    name = "value",
    visibility = ["//visibility:public"],
    deps = [
        ":value_table_parser",
        ":value_description_parser",
    ],
) 