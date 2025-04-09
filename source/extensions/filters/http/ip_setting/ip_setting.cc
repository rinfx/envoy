#include "source/extensions/filters/http/ip_setting/ip_setting.h"

#include <cmath>
#include <csignal>
#include <cstddef>
#include <string>

#include "envoy/extensions/filters/http/ip_setting/v3/ip_setting.pb.h"
#include "envoy/http/header_map.h"

#include "source/common/http/utility.h"
#include "source/common/protobuf/utility.h"
#include "source/common/json/json_internal.h"
#include "absl/strings/numbers.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace IPSetting {

using ::envoy::extensions::filters::http::ip_setting::v3::IPSetting;


FilterConfig::FilterConfig(const IPSetting& ip_setting, const std::string& stats_prefix, Stats::Scope& scope)
  : placeholder_(ip_setting.placeholder()), 
    scope_(scope),
    stat_name_pool_(scope_.symbolTable()),
    stats_(generateStats(stats_prefix, scope_)), 
    filter_stat_prefix_(stat_name_pool_.add("ip_setting")) {}


IPSettingFilter::IPSettingFilter(FilterConfigSharedPtr config, Upstream::ClusterManager& cm) 
  : config_(config), cm_(cm) {}

Http::FilterHeadersStatus IPSettingFilter::decodeHeaders(Http::RequestHeaderMap&, bool) {
  auto& upstream_cluster = decoder_callbacks_->clusterInfo()->name();
  // decoder_callbacks_->setUpstreamOverrideHost("127.0.0.1:6001");
  ENVOY_LOG(debug, "upstream cluster is {}", upstream_cluster);
  ProtobufWkt::Struct infos;
  auto& fields = *infos.mutable_fields();
  for (auto& p : cm_.getThreadLocalCluster(upstream_cluster)->prioritySet().hostSetsPerPriority()) {
    for (auto& h : p->hosts()) {
      ENVOY_LOG(debug, "endpoint: {}, metrics: {}", h->address()->asString(), h->getEndpointMetrics());
      *fields[h->address()->asString()].mutable_string_value() = h->getEndpointMetrics();
    }
  }
  decoder_callbacks_->streamInfo().setDynamicMetadata("envoy.filters.http.ip_setting", infos);
  return Http::FilterHeadersStatus::Continue;
}

Http::FilterDataStatus IPSettingFilter::decodeData(Buffer::Instance& /*data*/, bool /*end_stream*/) {
  return Http::FilterDataStatus::Continue;
}

Http::FilterHeadersStatus IPSettingFilter::encodeHeaders(Http::ResponseHeaderMap& /*headers*/, bool) {
  return Http::FilterHeadersStatus::Continue;
}

Http::FilterDataStatus IPSettingFilter::encodeData(Buffer::Instance& /*data*/, bool /*end_stream*/) {
  return Http::FilterDataStatus::Continue;
}

} // namespace IPSetting
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
