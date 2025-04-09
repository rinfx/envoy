#include "source/extensions/filters/http/ip_setting/config.h"

#include "envoy/extensions/filters/http/ip_setting/v3/ip_setting.pb.h"
#include "envoy/extensions/filters/http/ip_setting/v3/ip_setting.pb.validate.h"
#include "envoy/registry/registry.h"

#include "source/extensions/filters/http/ip_setting/ip_setting.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace IPSetting {

Http::FilterFactoryCb IPSettingFactory::createFilterFactoryFromProtoTyped(
    const envoy::extensions::filters::http::ip_setting::v3::IPSetting& proto_config,
    const std::string& stats_prefix, Server::Configuration::FactoryContext& context) {
  // Stats::StatNameManagedStorage prefix(stats_prefix, context.scope().symbolTable());
  FilterConfigSharedPtr config =
      std::make_unique<FilterConfig>(proto_config, stats_prefix, context.scope());
  return [config, &context](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    callbacks.addStreamFilter(std::make_shared<IPSettingFilter>(config, context.clusterManager()));
  };
}

/**
 * Static registration for the AIStatistic filter. @see RegisterFactory.
 */
REGISTER_FACTORY(IPSettingFactory, Server::Configuration::NamedHttpFilterConfigFactory);

} // namespace AIStatistic
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
