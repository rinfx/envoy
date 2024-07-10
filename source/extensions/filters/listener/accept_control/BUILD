load(
    "//bazel:envoy_build_system.bzl",
    "envoy_cc_extension",
    "envoy_cc_library",
    "envoy_extension_package",
)

licenses(["notice"])  # Apache 2

envoy_extension_package()

envoy_cc_library(
    name = "config_lib",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    deps = [
        "@envoy_api//envoy/extensions/filters/listener/accept_control/v3:pkg_cc_proto",    
        "//source/common/network:cidr_range_lib",
    ],
)

envoy_cc_library(
    name = "accept_control_lib",
    srcs = ["accept_control.cc"],
    hdrs = ["accept_control.h"],
    deps = [
        ":config_lib",
        "//envoy/buffer:buffer_interface",
        "//envoy/network:address_interface",
        "//envoy/network:connection_interface",
        "//envoy/network:filter_interface",
        "//source/common/common:assert_lib",
        "//source/common/common:minimal_logger_lib",
        "//source/common/network:cidr_range_lib",
    ],
)

envoy_cc_extension(
    name = "config",  # The extension build system requires a library named config
    srcs = ["config_factory.cc"],
    hdrs = ["config_factory.h"],
    deps = [
        ":config_lib",
        ":accept_control_lib",
        "//envoy/registry",
        "//envoy/server:filter_config_interface",
        "@envoy_api//envoy/extensions/filters/listener/accept_control/v3:pkg_cc_proto",
    ],
)
