cc_library(
    name = "attribute_definition_parser",
    srcs = ["attribute_definition_parser.cc"],
    hdrs = ["attribute_definition_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
)

cc_library(
    name = "attribute_definition_default_parser",
    srcs = ["attribute_definition_default_parser.cc"],
    hdrs = ["attribute_definition_default_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        ":attribute_definition_parser",
        "//third_party/pegtl:pegtl",
    ],
)

cc_library(
    name = "attribute_value_parser",
    srcs = ["attribute_value_parser.cc"],
    hdrs = ["attribute_value_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        ":attribute_definition_parser",
        "//third_party/pegtl:pegtl",
    ],
)

cc_library(
    name = "attribute",
    visibility = ["//visibility:public"],
    deps = [
        ":attribute_definition_parser",
        ":attribute_definition_default_parser",
        ":attribute_value_parser",
    ],
) 