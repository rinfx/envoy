#pragma once

#include "envoy/stats/stats_macros.h"

#include "envoy/extensions/filters/http/ip_setting/v3/ip_setting.pb.h"
#include "envoy/http/filter.h"
#include "envoy/http/header_map.h"
#include "envoy/buffer/buffer.h"
#include "source/common/buffer/buffer_impl.h"

#include "source/common/http/header_utility.h"
#include "source/common/http/headers.h"
#include <deque>
#include <sstream>

#include "source/common/http/async_client_impl.h"
#include "source/common/http/codes.h"
#include "envoy/tracing/tracer.h"
#include "source/common/common/enum_to_int.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace IPSetting {

/**
 * All AI Statistic filter stats. @see stats_macros.h
 */
#define ALL_IP_SETTING_STATS(COUNTER)                                                              \
  COUNTER(count)

/**
 * Struct definition for AI Statistic stats. @see stats_macros.h
 */
struct IPSettingStats {
  ALL_IP_SETTING_STATS(GENERATE_COUNTER_STRUCT)
};

/**
 * Configuration for IPSettingFilter.
 */
class FilterConfig {
public:
  FilterConfig(const envoy::extensions::filters::http::ip_setting::v3::IPSetting& ip_setting, 
    const std::string& stats_prefix, Stats::Scope& scope);

  const IPSettingStats& stats() const { return stats_; }
  const Stats::Scope& scope() const { return scope_; }
  Stats::StatNamePool& statNamePool() { return stat_name_pool_; }

private:
  static IPSettingStats generateStats(const std::string& prefix, Stats::Scope& scope) {
    return IPSettingStats{ALL_IP_SETTING_STATS(POOL_COUNTER_PREFIX(scope, prefix))};
  }

  std::string placeholder_;
  Stats::Scope& scope_;
  Stats::StatNamePool stat_name_pool_;
  IPSettingStats stats_;

public:
  const Stats::StatName filter_stat_prefix_;
};

using FilterConfigSharedPtr = std::shared_ptr<FilterConfig>;

/**
 * IPSettingFilter
 */
class IPSettingFilter : public Http::StreamFilter, Logger::Loggable<Logger::Id::filter> {
public:
  IPSettingFilter(FilterConfigSharedPtr, Upstream::ClusterManager&);

  ~IPSettingFilter() override = default;

  // Http::StreamFilterBase
  void onDestroy() override {}

  // Http::StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(Http::RequestHeaderMap&, bool) override;

  Http::FilterDataStatus decodeData(Buffer::Instance&, bool) override;

  Http::FilterTrailersStatus decodeTrailers(Http::RequestTrailerMap&) override {
    return Http::FilterTrailersStatus::Continue;
  }

  void setEncoderFilterCallbacks(Http::StreamEncoderFilterCallbacks& callbacks) override {
    encoder_callbacks_ = &callbacks;
  }

  void setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) override {
    decoder_callbacks_ = &callbacks;
  }

  // Http::StreamEncoderFilter
  Http::Filter1xxHeadersStatus encode1xxHeaders(Http::ResponseHeaderMap&) override {
    return Http::Filter1xxHeadersStatus::Continue;
  }

  Http::FilterHeadersStatus encodeHeaders(Http::ResponseHeaderMap&, bool) override;

  Http::FilterDataStatus encodeData(Buffer::Instance&, bool) override;

  Http::FilterTrailersStatus encodeTrailers(Http::ResponseTrailerMap&) override {
    return Http::FilterTrailersStatus::Continue;
  }

  Http::FilterMetadataStatus encodeMetadata(Http::MetadataMap& m) override {
    m.insert(std::make_pair("input_token", "20"));
    std::stringstream s;
    s << m;
    std::string metadata_str;
    s >> metadata_str;
    ENVOY_LOG(info, "metadata is {}", metadata_str);
    return Http::FilterMetadataStatus::Continue;
  }

private:
  FilterConfigSharedPtr config_;
  Upstream::ClusterManager& cm_;
  Http::StreamDecoderFilterCallbacks* decoder_callbacks_{};
  Http::StreamEncoderFilterCallbacks* encoder_callbacks_{};
};

} // namespace IPSetting
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
