#include "source/extensions/filters/http/http_filter_demo/config.h"

#include "envoy/extensions/filters/http/http_filter_demo/v3/http_filter_demo.pb.h"
#include "envoy/extensions/filters/http/http_filter_demo/v3/http_filter_demo.pb.validate.h"
#include "envoy/registry/registry.h"

#include "source/extensions/filters/http/http_filter_demo/http_filter_demo.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace HttpFilterDemo {

Http::FilterFactoryCb HttpFilterDemoFactory::createFilterFactoryFromProtoTyped(
    const envoy::extensions::filters::http::http_filter_demo::v3::HttpFilterDemo& proto_config,
    const std::string& stats_prefix, Server::Configuration::FactoryContext& context) {
  // Stats::StatNameManagedStorage prefix(stats_prefix, context.scope().symbolTable());
  FilterConfigSharedPtr config =
      std::make_unique<FilterConfig>(proto_config, stats_prefix, context.scope());
  return [config, &context](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    auto client = std::make_unique<RawHttpClientImpl>(context.clusterManager());
    callbacks.addStreamFilter(std::make_shared<HttpFilterDemoFilter>(config, std::move(client)));
  };
}

/**
 * Static registration for the AIStatistic filter. @see RegisterFactory.
 */
REGISTER_FACTORY(HttpFilterDemoFactory, Server::Configuration::NamedHttpFilterConfigFactory);

} // namespace AIStatistic
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
