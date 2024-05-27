#pragma once

#include "envoy/extensions/filters/http/ai_statistic/v3/ai_statistic.pb.h"
#include "envoy/extensions/filters/http/ai_statistic/v3/ai_statistic.pb.validate.h"

#include "source/extensions/filters/http/common/factory_base.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace AIStatistic {

/**
 * Config registration for AIStatisticFilter. @see NamedHttpFilterConfigFactory.
 */
class AIStatisticFilterFactory
    : public Common::FactoryBase<envoy::extensions::filters::http::ai_statistic::v3::AIStatistic> {
public:
  AIStatisticFilterFactory() : FactoryBase("envoy.filters.http.ai_statistic") {}

private:
  Http::FilterFactoryCb createFilterFactoryFromProtoTyped(
      const envoy::extensions::filters::http::ai_statistic::v3::AIStatistic& proto_config,
      const std::string& stats_prefix, Server::Configuration::FactoryContext& context) override;

  // Router::RouteSpecificFilterConfigConstSharedPtr createRouteSpecificFilterConfigTyped(
  //     const envoy::extensions::filters::http::ai_statistic::v3::AIStatistic& proto_config,
  //     Server::Configuration::ServerFactoryContext& context,
  //     ProtobufMessage::ValidationVisitor& validator) override;
};

} // namespace AIStatistic
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
