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
    scope_(scope), 
    stat_name_pool_(scope_.symbolTable()),
    stats_(generateStats(stats_prefix, scope_)), 
    filter_stat_prefix_(stat_name_pool_.add("ai_statistic")),
    route_(stat_name_pool_.add("route")),
    input_token_(stat_name_pool_.add("input_token")),
    output_token_(stat_name_pool_.add("output_token")) {}


AIStatisticFilter::AIStatisticFilter(FilterConfigSharedPtr config) 
  : config_(config) {}

Http::FilterDataStatus AIStatisticFilter::decodeData(Buffer::Instance& data, bool end_stream) {
  if (!end_stream) {
    decoder_callbacks_->addDecodedData(data, false);
    return Http::FilterDataStatus::StopIterationAndBuffer;
  }

  decoder_callbacks_->addDecodedData(data, false);
  const Buffer::Instance& decoding_buffer = *decoder_callbacks_->decodingBuffer();
  auto jsonObj = Json::Nlohmann::Factory::loadFromString(decoding_buffer.toString());
  model_ = jsonObj->getString("model");
  return Http::FilterDataStatus::Continue;
}

Http::FilterDataStatus AIStatisticFilter::encodeData(Buffer::Instance& data, bool end_stream) {
  if (end_stream) {
    // global metrics
    config_->stats().input_token_.add(input_tokens_);
    config_->stats().output_token_.add(output_tokens_);
    // route-level metrics
    if (encoder_callbacks_->streamInfo().route()->routeEntry()) {
      config_->addCounter({config_->filter_stat_prefix_,
        config_->route_,
        encoder_callbacks_->streamInfo().route()->routeEntry()->routeStatsContext()->statName(),
        config_->input_token_}, input_tokens_);
      config_->addCounter({config_->filter_stat_prefix_,
        config_->route_,
        encoder_callbacks_->streamInfo().route()->routeEntry()->routeStatsContext()->statName(),
        config_->output_token_}, output_tokens_);
    }
    // cluster-level metrics
    if (encoder_callbacks_->clusterInfo()) {
      config_->addCounter({config_->filter_stat_prefix_,
        encoder_callbacks_->clusterInfo()->statsScope().prefix(),
        config_->input_token_}, input_tokens_);
      config_->addCounter({config_->filter_stat_prefix_,
        encoder_callbacks_->clusterInfo()->statsScope().prefix(),
        config_->output_token_}, output_tokens_);
    }
    // model-level metrics
    if (!model_.empty()) {
      config_->addCounter({config_->filter_stat_prefix_,
        config_->statNamePool().add("model"),
        config_->statNamePool().add(model_),
        config_->input_token_}, input_tokens_);
      config_->addCounter({config_->filter_stat_prefix_,
        config_->statNamePool().add("model"),
        config_->statNamePool().add(model_),
        config_->output_token_}, output_tokens_);
    }
    return Http::FilterDataStatus::Continue;
  }

  std::string raw_str = data.toString();
  
  const std::string input_token_marker = "\"input_tokens\":";
  size_t pos1 = raw_str.find(input_token_marker)+input_token_marker.size();
  int input_tokens=0;
  for(size_t i=pos1; i<raw_str.size(); i++) {
    if(raw_str[i]-'0'<0 || raw_str[i]-'0'>9) {
      break;
    }
    input_tokens = input_tokens * 10 + raw_str[i]-'0';
  }
  input_tokens_ = input_tokens;

  const std::string output_token_marker = ",\"output_tokens\":";
  size_t pos2 = raw_str.find(output_token_marker)+output_token_marker.size();
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
