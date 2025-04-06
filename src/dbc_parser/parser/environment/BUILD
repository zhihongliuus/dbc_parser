cc_library(
    name = "environment_variable_parser",
    srcs = ["environment_variable_parser.cc"],
    hdrs = ["environment_variable_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "@taocpp_pegtl//:pegtl",
    ],
)

cc_library(
    name = "environment_variable_data_parser",
    srcs = ["environment_variable_data_parser.cc"],
    hdrs = ["environment_variable_data_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "@taocpp_pegtl//:pegtl",
    ],
)

cc_library(
    name = "environment",
    visibility = ["//visibility:public"],
    deps = [
        ":environment_variable_parser",
        ":environment_variable_data_parser",
    ],
) 