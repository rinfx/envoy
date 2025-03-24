#pragma once

#include "envoy/stats/stats_macros.h"

#include "envoy/extensions/filters/http/http_filter_demo/v3/http_filter_demo.pb.h"
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
namespace HttpFilterDemo {

/**
 * Async callbacks used during fetchToken() calls.
 */
class RequestCallbacks {
public:
  virtual ~RequestCallbacks() = default;

  /**
   * Called on completion of request.
   *
   * @param response the pointer to the response message. Null response pointer means the request
   *        was completed with error.
   */
  virtual void onComplete(const Http::ResponseMessage* response_ptr) PURE;
};

class RawHttpClientImpl : public Http::AsyncClient::Callbacks,
                          Logger::Loggable<Logger::Id::config> {
public:
  explicit RawHttpClientImpl(Upstream::ClusterManager& cm): cm_(cm) {}
  ~RawHttpClientImpl() override {
    ASSERT(callbacks_ == nullptr);
  }

  void call(RequestCallbacks& callbacks);
  void resetCallbacks();
  void cancel();
  // Http::AsyncClient::Callbacks
  void onSuccess(const Http::AsyncClient::Request&, Http::ResponseMessagePtr&& response) override;
  void onFailure(const Http::AsyncClient::Request&, Http::AsyncClient::FailureReason reason) override;

  void onBeforeFinalizeUpstreamSpan(Tracing::Span&, const Http::ResponseHeaderMap*) override {}

  RequestCallbacks* callbacks_{};
private:
  Upstream::ClusterManager& cm_;
  Http::AsyncClient::Request* request_{};
};

using ClientPtr = std::unique_ptr<RawHttpClientImpl>;

/**
 * All AI Statistic filter stats. @see stats_macros.h
 */
#define ALL_HTTP_FILTER_DEMO_STATS(COUNTER)                                                              \
  COUNTER(count)

/**
 * Struct definition for AI Statistic stats. @see stats_macros.h
 */
struct HttpFilterDemoStats {
  ALL_HTTP_FILTER_DEMO_STATS(GENERATE_COUNTER_STRUCT)
};

/**
 * Configuration for HttpFilterDemoFilter.
 */
class FilterConfig {
public:
  FilterConfig(const envoy::extensions::filters::http::http_filter_demo::v3::HttpFilterDemo& http_filter_demo, 
    const std::string& stats_prefix, Stats::Scope& scope);

  const HttpFilterDemoStats& stats() const { return stats_; }
  const Stats::Scope& scope() const { return scope_; }
  Stats::StatNamePool& statNamePool() { return stat_name_pool_; }

  void addCounter(const Stats::StatNameVec& names, int delta) const {
    const Stats::SymbolTable::StoragePtr stat_name_storage = scope_.symbolTable().join(names);
    scope_.counterFromStatName(Stats::StatName(stat_name_storage.get())).add(delta);
  }

private:
  static HttpFilterDemoStats generateStats(const std::string& prefix, Stats::Scope& scope) {
    return HttpFilterDemoStats{ALL_HTTP_FILTER_DEMO_STATS(POOL_COUNTER_PREFIX(scope, prefix))};
  }

  std::string placeholder_;
  Stats::Scope& scope_;
  Stats::StatNamePool stat_name_pool_;
  HttpFilterDemoStats stats_;

public:
  const Stats::StatName filter_stat_prefix_;
  const Stats::StatName route_;
  const Stats::StatName count_;
};

using FilterConfigSharedPtr = std::shared_ptr<FilterConfig>;

/**
 * HttpFilterDemoFilter
 */
class HttpFilterDemoFilter : public Http::StreamFilter, public RequestCallbacks,
                      Logger::Loggable<Logger::Id::filter> {
public:
  HttpFilterDemoFilter(FilterConfigSharedPtr, ClientPtr&&);

  ~HttpFilterDemoFilter() override = default;

  // Http::StreamFilterBase
  void onDestroy() override {}

  // Http::StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(Http::RequestHeaderMap&, bool) override {
    return Http::FilterHeadersStatus::Continue;
  }

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

  void onComplete(const Http::ResponseMessage* response_ptr) override;

private:
  FilterConfigSharedPtr config_;
  std::deque<Buffer::OwnedImpl> chunk_queue_;
  bool end_stream_{};
  bool during_call_{};
  Http::StreamDecoderFilterCallbacks* decoder_callbacks_{};
  Http::StreamEncoderFilterCallbacks* encoder_callbacks_{};
  ClientPtr client_;
};

} // namespace HttpFilterDemo
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
