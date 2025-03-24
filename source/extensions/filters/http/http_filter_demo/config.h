#pragma once

#include "envoy/extensions/filters/http/http_filter_demo/v3/http_filter_demo.pb.h"
#include "envoy/extensions/filters/http/http_filter_demo/v3/http_filter_demo.pb.validate.h"

#include "source/extensions/filters/http/common/factory_base.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace HttpFilterDemo {

/**
 * Config registration for HttpFilterDemoFilter. @see NamedHttpFilterConfigFactory.
 */
class HttpFilterDemoFactory
    : public Common::FactoryBase<envoy::extensions::filters::http::http_filter_demo::v3::HttpFilterDemo> {
public:
  HttpFilterDemoFactory() : FactoryBase("envoy.filters.http.http_filter_demo") {}

private:
  Http::FilterFactoryCb createFilterFactoryFromProtoTyped(
      const envoy::extensions::filters::http::http_filter_demo::v3::HttpFilterDemo& proto_config,
      const std::string& stats_prefix, Server::Configuration::FactoryContext& context) override;

  // Router::RouteSpecificFilterConfigConstSharedPtr createRouteSpecificFilterConfigTyped(
  //     const envoy::extensions::filters::http::http_filter_demo::v3::HttpFilterDemo& proto_config,
  //     Server::Configuration::ServerFactoryContext& context,
  //     ProtobufMessage::ValidationVisitor& validator) override;
};

} // namespace HttpFilterDemo
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
