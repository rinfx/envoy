#pragma once

#include "envoy/extensions/filters/http/ip_setting/v3/ip_setting.pb.h"
#include "envoy/extensions/filters/http/ip_setting/v3/ip_setting.pb.validate.h"

#include "source/extensions/filters/http/common/factory_base.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace IPSetting {

/**
 * Config registration for IPSettingFilter. @see NamedHttpFilterConfigFactory.
 */
class IPSettingFactory
    : public Common::FactoryBase<envoy::extensions::filters::http::ip_setting::v3::IPSetting> {
public:
  IPSettingFactory() : FactoryBase("envoy.filters.http.ip_setting") {}

private:
  Http::FilterFactoryCb createFilterFactoryFromProtoTyped(
      const envoy::extensions::filters::http::ip_setting::v3::IPSetting& proto_config,
      const std::string& stats_prefix, Server::Configuration::FactoryContext& context) override;

  // Router::RouteSpecificFilterConfigConstSharedPtr createRouteSpecificFilterConfigTyped(
  //     const envoy::extensions::filters::http::ip_setting::v3::IPSetting& proto_config,
  //     Server::Configuration::ServerFactoryContext& context,
  //     ProtobufMessage::ValidationVisitor& validator) override;
};

} // namespace IPSetting
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
