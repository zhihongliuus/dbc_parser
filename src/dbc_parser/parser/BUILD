cc_library(
    name = "version_parser",
    srcs = ["version_parser.cc"],
    hdrs = ["version_parser.h"],
    deps = [
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "new_symbols_parser",
    srcs = ["new_symbols_parser.cc"],
    hdrs = ["new_symbols_parser.h"],
    deps = [
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "bit_timing_parser",
    srcs = ["bit_timing_parser.cc"],
    hdrs = ["bit_timing_parser.h"],
    deps = [
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "nodes_parser",
    srcs = ["nodes_parser.cc"],
    hdrs = ["nodes_parser.h"],
    deps = [
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "value_table_parser",
    srcs = ["value_table_parser.cc"],
    hdrs = ["value_table_parser.h"],
    deps = [
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "message_parser",
    srcs = ["message_parser.cc"],
    hdrs = ["message_parser.h"],
    deps = [
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "message_transmitters_parser",
    srcs = ["message_transmitters_parser.cc"],
    hdrs = ["message_transmitters_parser.h"],
    deps = [
        "//src/dbc_parser/core:string_utils",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "environment_variable_parser",
    srcs = ["environment_variable_parser.cc"],
    hdrs = ["environment_variable_parser.h"],
    deps = [
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "environment_variable_data_parser",
    srcs = ["environment_variable_data_parser.cc"],
    hdrs = ["environment_variable_data_parser.h"],
    deps = [
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "signal_type_def_parser",
    srcs = ["signal_type_def_parser.cc"],
    hdrs = ["signal_type_def_parser.h"],
    deps = [
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "signal_value_type_parser",
    srcs = ["signal_value_type_parser.cc"],
    hdrs = ["signal_value_type_parser.h"],
    deps = [
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "signal_group_parser",
    srcs = ["signal_group_parser.cc"],
    hdrs = ["signal_group_parser.h"],
    deps = [
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "signal_parser",
    srcs = ["signal_parser.cc"],
    hdrs = ["signal_parser.h"],
    deps = [
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "comment_parser",
    srcs = ["comment_parser.cc"],
    hdrs = ["comment_parser.h"],
    deps = [
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "value_description_parser",
    srcs = ["value_description_parser.cc"],
    hdrs = ["value_description_parser.h"],
    deps = [
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "attribute_definition_parser",
    srcs = ["attribute_definition_parser.cc"],
    hdrs = ["attribute_definition_parser.h"],
    deps = [
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "attribute_definition_default_parser",
    srcs = ["attribute_definition_default_parser.cc"],
    hdrs = ["attribute_definition_default_parser.h"],
    deps = [
        ":attribute_definition_parser",
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "attribute_value_parser",
    srcs = ["attribute_value_parser.cc"],
    hdrs = ["attribute_value_parser.h"],
    deps = [
        ":attribute_definition_parser",
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
)

# DBC File Parser - main parser that integrates all others
cc_library(
    name = "dbc_file_parser",
    srcs = ["dbc_file_parser.cc"],
    hdrs = ["dbc_file_parser.h"],
    deps = [
        ":version_parser",
        ":new_symbols_parser",
        ":nodes_parser",
        ":message_parser",
        ":message_transmitters_parser",
        "//third_party/pegtl:pegtl",
    ],
    visibility = ["//visibility:public"],
) 