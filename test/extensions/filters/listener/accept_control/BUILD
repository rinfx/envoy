load(
    "//bazel:envoy_build_system.bzl",
    "envoy_package",
)
load(
    "//test/extensions:extensions_build_system.bzl",
    "envoy_extension_cc_test",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_extension_cc_test(
    name = "accept_control_config_test",
    srcs = ["accept_control_config_test.cc"],
    extension_names = ["envoy.filters.listener.accept_control"],
    deps = [
        "//source/extensions/filters/listener/accept_control:config_lib",
        "//source/common/network:cidr_range_lib",
        "//source/common/network:address_lib",
        "//test/test_common:utility_lib",
        "//test/mocks/stats:stats_mocks",
        "@envoy_api//envoy/extensions/filters/listener/accept_control/v3:pkg_cc_proto",
    ],
)

envoy_extension_cc_test(
    name = "accept_control_config_factory_test",
    srcs = ["accept_control_config_factory_test.cc"],
    extension_names = ["envoy.filters.listener.accept_control"],
    deps = [
        "//source/extensions/filters/listener/accept_control:config",
        "//source/extensions/filters/listener/accept_control:config_lib",
        "//source/extensions/filters/listener/accept_control:accept_control_lib",
        "//test/mocks/server:listener_factory_context_mocks",
    ],
)

envoy_extension_cc_test(
    name = "accept_control_test",
    srcs = ["accept_control_test.cc"],
    extension_names = ["envoy.filters.listener.accept_control"],
    deps = [
        "//source/common/network:socket_option_lib",
        "//source/common/network:listener_filter_buffer_lib",
        "//source/extensions/filters/listener/accept_control:accept_control_lib",
        "//test/mocks:common_lib",
        "//test/mocks/buffer:buffer_mocks",
        "//test/mocks/network:network_mocks",
        "//test/test_common:printers_lib",
        "//test/mocks/stats:stats_mocks",
        "@envoy_api//envoy/config/core/v3:pkg_cc_proto",
        "@envoy_api//envoy/extensions/filters/listener/accept_control/v3:pkg_cc_proto",
    ],
)