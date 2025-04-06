cc_library(
    name = "string_utils",
    srcs = ["string_utils.cc"],
    hdrs = ["string_utils.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "logger",
    srcs = [
        "logger.cc",
        "log_functions.cc",
    ],
    hdrs = [
        "logger.h",
        "log_macros.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@spdlog//:spdlog",
        "@fmt//:fmt",
    ],
)

cc_library(
    name = "core",
    visibility = ["//visibility:public"],
    deps = [
        ":string_utils",
        ":logger",
    ],
)

cc_binary(
    name = "logging_example",
    srcs = ["logging_example.cc"],
    deps = [
        ":logger",
    ],
) 