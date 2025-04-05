cc_library(
    name = "message_parser",
    srcs = ["message_parser.cc"],
    hdrs = ["message_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
)

cc_library(
    name = "message_transmitters_parser",
    srcs = ["message_transmitters_parser.cc"],
    hdrs = ["message_transmitters_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
)

cc_library(
    name = "signal_parser",
    srcs = ["signal_parser.cc"],
    hdrs = ["signal_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
)

cc_library(
    name = "signal_group_parser",
    srcs = ["signal_group_parser.cc"],
    hdrs = ["signal_group_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
)

cc_library(
    name = "signal_type_def_parser",
    srcs = ["signal_type_def_parser.cc"],
    hdrs = ["signal_type_def_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
)

cc_library(
    name = "signal_value_type_parser",
    srcs = ["signal_value_type_parser.cc"],
    hdrs = ["signal_value_type_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/dbc_parser/common:common",
        "//src/dbc_parser/core:string_utils",
        "//third_party/pegtl:pegtl",
    ],
)

cc_library(
    name = "message",
    visibility = ["//visibility:public"],
    deps = [
        ":message_parser",
        ":message_transmitters_parser",
        ":signal_parser",
        ":signal_group_parser",
        ":signal_type_def_parser",
        ":signal_value_type_parser",
    ],
) 