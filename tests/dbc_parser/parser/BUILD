cc_test(
    name = "version_parser_test",
    srcs = ["version_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:version_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "new_symbols_parser_test",
    srcs = ["new_symbols_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:new_symbols_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "bit_timing_parser_test",
    srcs = ["bit_timing_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:bit_timing_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "nodes_parser_test",
    srcs = ["nodes_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:nodes_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "value_table_parser_test",
    srcs = ["value_table_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:value_table_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "message_parser_test",
    srcs = ["message_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:message_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "message_transmitters_parser_test",
    srcs = ["message_transmitters_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:message_transmitters_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "environment_variable_parser_test",
    srcs = ["environment_variable_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:environment_variable_parser",
        "@com_google_googletest//:gtest_main"
    ],
)

cc_test(
    name = "environment_variable_data_parser_test",
    srcs = ["environment_variable_data_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:environment_variable_data_parser",
        "@com_google_googletest//:gtest_main"
    ],
)

cc_test(
    name = "signal_type_def_parser_test",
    srcs = ["signal_type_def_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:signal_type_def_parser",
        "@com_google_googletest//:gtest_main"
    ],
)

cc_test(
    name = "signal_value_type_parser_test",
    srcs = ["signal_value_type_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:signal_value_type_parser",
        "@com_google_googletest//:gtest_main"
    ],
)

cc_test(
    name = "signal_group_parser_test",
    srcs = ["signal_group_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:signal_group_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

# Test for the signal parser
cc_test(
    name = "signal_parser_test",
    srcs = ["signal_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:signal_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

# Test for the comment parser
cc_test(
    name = "comment_parser_test",
    srcs = ["comment_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:comment_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

# Test for the value description parser
cc_test(
    name = "value_description_parser_test",
    srcs = ["value_description_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:value_description_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "attribute_definition_parser_test",
    srcs = ["attribute_definition_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:attribute_definition_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "attribute_definition_default_parser_test",
    srcs = ["attribute_definition_default_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:attribute_definition_default_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "attribute_value_parser_test",
    srcs = ["attribute_value_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:attribute_value_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

# Test for the DBC file parser
cc_test(
    name = "dbc_file_parser_test",
    srcs = ["dbc_file_parser_test.cc"],
    deps = [
        "//src/dbc_parser/parser:dbc_file_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

test_suite(
    name = "all",
    tests = [
        ":bit_timing_parser_test",
        ":comment_parser_test",
        ":dbc_file_parser_test",
        ":environment_variable_data_parser_test",
        ":environment_variable_parser_test",
        ":message_parser_test",
        ":message_transmitters_parser_test",
        ":new_symbols_parser_test",
        ":nodes_parser_test",
        ":signal_group_parser_test",
        ":signal_parser_test",
        ":signal_type_def_parser_test",
        ":signal_value_type_parser_test",
        ":value_description_parser_test",
        ":value_table_parser_test",
        ":version_parser_test",
        ":attribute_definition_parser_test",
        ":attribute_definition_default_parser_test",
        ":attribute_value_parser_test",
    ],
) 