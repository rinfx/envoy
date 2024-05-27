load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_extension",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "ai_statistic_filter_lib",
    srcs = ["ai_statistic_filter.cc"],
    hdrs = ["ai_statistic_filter.h"],
    deps = [
        "//envoy/http:filter_interface",
        "//envoy/http:header_map_interface",
        "//source/common/http:header_map_lib",
        "//source/common/http:header_utility_lib",
        "//source/common/http:headers_lib",
        "//source/common/protobuf:utility_lib",
        "//source/common/json:json_internal_lib",
        "@envoy_api//envoy/extensions/filters/http/ai_statistic/v3:pkg_cc_proto",
    ],
)

envoy_cc_extension(
    name = "ai_statistic_config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    deps = [
        "//envoy/registry",
        "//source/extensions/filters/http/common:factory_base_lib",
        "//source/extensions/filters/http/ai_statistic:ai_statistic_filter_lib",
        "@envoy_api//envoy/extensions/filters/http/ai_statistic/v3:pkg_cc_proto",
    ],
)
