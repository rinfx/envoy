#include "source/extensions/filters/http/ai_statistic/ai_statistic_filter.h"

#include <cmath>
#include <csignal>
#include <cstddef>
#include <string>

#include "ai_statistic_filter.h"
#include "envoy/extensions/filters/http/ai_statistic/v3/ai_statistic.pb.h"
#include "envoy/http/header_map.h"

#include "source/common/http/utility.h"
#include "source/common/protobuf/utility.h"
#include "source/common/json/json_internal.h"
#include "absl/strings/numbers.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace AIStatistic {

using ::envoy::extensions::filters::http::ai_statistic::v3::AIStatistic;

FilterConfig::FilterConfig(const AIStatistic& ai_statistic, const std::string& stats_prefix, Stats::Scope& scope)
  : placeholder_(ai_statistic.placeholder()), 
    scope_(scope.createScope("ai_statistic")), 
    stat_name_pool_(scope_->symbolTable()),
    stats_(generateStats(stats_prefix, *scope_)) {}


AIStatisticFilter::AIStatisticFilter(FilterConfigSharedPtr config) 
  : config_(config), 
  route_(config->statNamePool().add("route")),
  input_token_(config->statNamePool().add("input_token")),
  output_token_(config->statNamePool().add("output_token")) {}

Http::FilterHeadersStatus AIStatisticFilter::decodeHeaders(Http::RequestHeaderMap&, bool) {
  return Http::FilterHeadersStatus::Continue;
}

Http::FilterDataStatus AIStatisticFilter::encodeData(Buffer::Instance& data, bool end_stream) {
  // if (!end_stream) {
  //   return Http::FilterDataStatus::StopIterationAndBuffer;
  // }

  // encoder_callbacks_->addEncodedData(data, false);
  // const Buffer::Instance& encoding_buffer = *encoder_callbacks_->encodingBuffer();
  // ENVOY_LOG(info, "AIStatisticFilter response body: {}", encoding_buffer.toString());
  if (end_stream) {
    config_->stats().input_token_.add(input_tokens_);
    config_->stats().output_token_.add(output_tokens_);
    config_->addCounter({route_,
      encoder_callbacks_->streamInfo().route()->routeEntry()->routeStatsContext()->statName(),
      input_token_}, input_tokens_);
    config_->addCounter({route_,
      encoder_callbacks_->streamInfo().route()->routeEntry()->routeStatsContext()->statName(),
      output_token_}, output_tokens_);
    config_->addCounter({encoder_callbacks_->clusterInfo()->statsScope().prefix(),
      input_token_}, input_tokens_);
    return Http::FilterDataStatus::Continue;
  }

  std::string raw_str = data.toString();
  const std::string input_token_marker = "\"input_tokens\":";
  const std::string output_token_marker = ",\"output_tokens\":";
  
  size_t pos1 = raw_str.find(input_token_marker)+input_token_marker.size();
  size_t pos2 = raw_str.find(output_token_marker)+output_token_marker.size();
  int input_tokens=0;
  for(size_t i=pos1; i<raw_str.size(); i++) {
    if(raw_str[i]-'0'<0 || raw_str[i]-'0'>9) {
      break;
    }
    input_tokens = input_tokens * 10 + raw_str[i]-'0';
  }
  input_tokens_ = input_tokens;
  int output_tokens=0;
  for(size_t i=pos2; i<raw_str.size(); i++) {
    if(raw_str[i]-'0'<0 || raw_str[i]-'0'>9) {
      break;
    }
    output_tokens = output_tokens * 10 + raw_str[i]-'0';
  }
  output_tokens_ = output_tokens;
  
  ENVOY_LOG(info, "AIStatisticFilter input tokens {}, output tokens {}", input_tokens, output_tokens);
  ENVOY_LOG(info, "AIStatisticFilter response body: {}", data.toString());
  return Http::FilterDataStatus::Continue;
}

} // namespace AIStatistic
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
