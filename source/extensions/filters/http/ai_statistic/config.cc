#include "source/extensions/filters/http/ai_statistic/config.h"

#include "envoy/extensions/filters/http/ai_statistic/v3/ai_statistic.pb.h"
#include "envoy/extensions/filters/http/ai_statistic/v3/ai_statistic.pb.validate.h"
#include "envoy/registry/registry.h"

#include "source/extensions/filters/http/ai_statistic/ai_statistic_filter.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace AIStatistic {

Http::FilterFactoryCb AIStatisticFilterFactory::createFilterFactoryFromProtoTyped(
    const envoy::extensions::filters::http::ai_statistic::v3::AIStatistic& proto_config,
    const std::string& stats_prefix, Server::Configuration::FactoryContext& context) {
  FilterConfigSharedPtr config =
      std::make_unique<FilterConfig>(proto_config, stats_prefix, context.scope());
  return [config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    callbacks.addStreamFilter(std::make_shared<AIStatisticFilter>(config));
  };
}

/**
 * Static registration for the AIStatistic filter. @see RegisterFactory.
 */
REGISTER_FACTORY(AIStatisticFilterFactory, Server::Configuration::NamedHttpFilterConfigFactory);

} // namespace AIStatistic
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
