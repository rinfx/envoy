#pragma once

#include "envoy/stats/stats_macros.h"

#include "envoy/extensions/filters/http/ai_statistic/v3/ai_statistic.pb.h"
#include "envoy/http/filter.h"
#include "envoy/http/header_map.h"

#include "source/common/http/header_utility.h"
#include "source/common/http/headers.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace AIStatistic {

/**
 * All AI Statistic filter stats. @see stats_macros.h
 */
#define ALL_AI_STATISTIC_STATS(COUNTER)                                                              \
  COUNTER(input_token)                                                                               \
  COUNTER(output_token)

/**
 * Struct definition for AI Statistic stats. @see stats_macros.h
 */
struct AIStatisticStats {
  ALL_AI_STATISTIC_STATS(GENERATE_COUNTER_STRUCT)
};

/**
 * Configuration for AIStatisticFilter.
 */
class FilterConfig {
public:
  FilterConfig(const envoy::extensions::filters::http::ai_statistic::v3::AIStatistic& ai_statistic, 
    const std::string& stats_prefix, Stats::Scope& scope);

  const AIStatisticStats& stats() const { return stats_; }
  const Stats::ScopeSharedPtr scope() const { return scope_; }
  Stats::StatNameDynamicPool& statNamePool() { return stat_name_pool_; }

  void addCounter(const Stats::StatNameVec& names, int delta) const {
    const Stats::SymbolTable::StoragePtr stat_name_storage = scope_->symbolTable().join(names);
    scope_->counterFromStatName(Stats::StatName(stat_name_storage.get())).add(delta);
  }

private:
  static AIStatisticStats generateStats(const std::string& prefix, Stats::Scope& scope) {
    return AIStatisticStats{ALL_AI_STATISTIC_STATS(POOL_COUNTER_PREFIX(scope, prefix))};
  }

  std::string placeholder_;
  // Stats::Scope& scope_;
  Stats::ScopeSharedPtr scope_;
  Stats::StatNameDynamicPool stat_name_pool_;
  AIStatisticStats stats_;
};

using FilterConfigSharedPtr = std::shared_ptr<FilterConfig>;

/**
 * AIStatisticFilter
 */
class AIStatisticFilter : public Http::StreamFilter, Logger::Loggable<Logger::Id::filter> {
public:
  AIStatisticFilter(FilterConfigSharedPtr);

  ~AIStatisticFilter() override = default;

  // Http::StreamFilterBase
  void onDestroy() override {}

  // Http::StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(Http::RequestHeaderMap&, bool) override;

  Http::FilterDataStatus decodeData(Buffer::Instance&, bool) override {
    return Http::FilterDataStatus::Continue;
  }

  Http::FilterTrailersStatus decodeTrailers(Http::RequestTrailerMap&) override {
    return Http::FilterTrailersStatus::Continue;
  }

  void setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) override {
    decoder_callbacks_ = &callbacks;
  }

  // Http::StreamEncoderFilter
  Http::Filter1xxHeadersStatus encode1xxHeaders(Http::ResponseHeaderMap&) override {
    return Http::Filter1xxHeadersStatus::Continue;
  }

  Http::FilterHeadersStatus encodeHeaders(Http::ResponseHeaderMap&, bool) override {
    return Http::FilterHeadersStatus::Continue;
  }

  Http::FilterDataStatus encodeData(Buffer::Instance&, bool) override;

  Http::FilterTrailersStatus encodeTrailers(Http::ResponseTrailerMap&) override {
    return Http::FilterTrailersStatus::Continue;
  }

  Http::FilterMetadataStatus encodeMetadata(Http::MetadataMap&) override {
    return Http::FilterMetadataStatus::Continue;
  }

  void setEncoderFilterCallbacks(Http::StreamEncoderFilterCallbacks& callbacks) override {
    encoder_callbacks_ = &callbacks;
  }

private:
  FilterConfigSharedPtr config_;
  Http::StreamDecoderFilterCallbacks* decoder_callbacks_{};
  Http::StreamEncoderFilterCallbacks* encoder_callbacks_{};
  const Stats::StatName route_;
  const Stats::StatName input_token_;
  const Stats::StatName output_token_;
  int input_tokens_=0;
  int output_tokens_=0;
};

} // namespace AIStatistic
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
