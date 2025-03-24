load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_extension",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "http_filter_demo_lib",
    srcs = ["http_filter_demo.cc"],
    hdrs = ["http_filter_demo.h"],
    deps = [
        "//envoy/buffer:buffer_interface",
        "//envoy/http:filter_interface",
        "//envoy/http:header_map_interface",
        "//source/common/http:header_map_lib",
        "//source/common/http:header_utility_lib",
        "//source/common/http:headers_lib",
        "//source/common/protobuf:utility_lib",
        "//source/common/json:json_internal_lib",
        "//source/common/http:async_client_lib",
        "//source/common/http:codes_lib",
        "//source/common/tracing:http_tracer_lib",
        "@envoy_api//envoy/extensions/filters/http/http_filter_demo/v3:pkg_cc_proto",
    ],
)

envoy_cc_extension(
    name = "config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    deps = [
        "//envoy/registry",
        "//source/extensions/filters/http/common:factory_base_lib",
        "//source/extensions/filters/http/http_filter_demo:http_filter_demo_lib",
        "@envoy_api//envoy/extensions/filters/http/http_filter_demo/v3:pkg_cc_proto",
    ],
)
