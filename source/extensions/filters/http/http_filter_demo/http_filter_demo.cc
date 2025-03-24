#include "source/extensions/filters/http/http_filter_demo/http_filter_demo.h"

#include <cmath>
#include <csignal>
#include <cstddef>
#include <string>

#include "envoy/extensions/filters/http/http_filter_demo/v3/http_filter_demo.pb.h"
#include "envoy/http/header_map.h"

#include "source/common/http/utility.h"
#include "source/common/protobuf/utility.h"
#include "source/common/json/json_internal.h"
#include "absl/strings/numbers.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace HttpFilterDemo {

using ::envoy::extensions::filters::http::http_filter_demo::v3::HttpFilterDemo;

void RawHttpClientImpl::resetCallbacks() {
  callbacks_ = nullptr;
}

void RawHttpClientImpl::call(RequestCallbacks& callbacks) {
  // ASSERT(callbacks_ == nullptr);
  // callbacks_ = &callbacks;
  if (!callbacks_) {
    callbacks_ = &callbacks;
  }
  const std::string& cluster = "outbound|6000||flask.static";
  const auto thread_local_cluster = cm_.getThreadLocalCluster(cluster);
  Http::RequestHeaderMapPtr headers;
  // const std::string& http_request_body = "helloworld";
  headers = Http::createHeaderMap<Http::RequestHeaderMapImpl>({
    {Http::Headers::get().ContentLength, std::to_string(0)},
    {Http::Headers::get().Method, Http::Headers::get().MethodValues.Get},
    // {Http::Headers::get().ContentLength, std::to_string(http_request_body.length())},
    {Http::Headers::get().Protocol, Http::Headers::get().ProtocolStrings.Http11String},
    {Http::Headers::get().Scheme, Http::Headers::get().SchemeValues.Http},
    {Http::Headers::get().Host, "127.0.0.1:6000"},
    {Http::Headers::get().Path, "/print_request"}});

  Http::RequestMessagePtr message =
    std::make_unique<Envoy::Http::RequestMessageImpl>(std::move(headers));
  // message->body().add(http_request_body);
  auto options = Http::AsyncClient::RequestOptions();
  options.setTimeout(std::chrono::milliseconds(5000));
  request_ = thread_local_cluster->httpAsyncClient().send(std::move(message), *this, options);
}

void RawHttpClientImpl::cancel() {
  ASSERT(callbacks_ != nullptr);
  request_->cancel();
  // callbacks_ = nullptr;
}

void RawHttpClientImpl::onSuccess(const Http::AsyncClient::Request&, Http::ResponseMessagePtr&& response) {
  auto status = Envoy::Http::Utility::getResponseStatusOrNullopt(response->headers());
  if (status.has_value()) {
    uint64_t status_code = status.value();
    if (status_code == Envoy::enumToInt(Envoy::Http::Code::OK)) {
      ASSERT(callbacks_ != nullptr);
      callbacks_->onComplete(response.get());
      // callbacks_ = nullptr;
    } else {
      ENVOY_LOG(error, "Response status is not OK, status: {}", status_code);
      cancel();
      ASSERT(callbacks_ != nullptr);
      callbacks_->onComplete(/*response_ptr=*/nullptr);
      // callbacks_ = nullptr;
    }
  } else {
    // This occurs if the response headers are invalid.
    ENVOY_LOG(error, "Failed to get the response because response headers are not valid.");
    cancel();
    ASSERT(callbacks_ != nullptr);
    callbacks_->onComplete(/*response_ptr=*/nullptr);
    // callbacks_ = nullptr;
  }
}

void RawHttpClientImpl::onFailure(const Http::AsyncClient::Request&, Http::AsyncClient::FailureReason reason) {
  // TODO(botengyao): handle different failure reasons.
  ASSERT(reason == Http::AsyncClient::FailureReason::Reset ||
        reason == Http::AsyncClient::FailureReason::ExceedResponseBufferLimit);
  Envoy::Http::ResponseHeaderMapPtr response_headers;
  response_headers->setStatus(403);
  const std::string body_text = "This is a simple response";
  Envoy::Buffer::OwnedImpl response_body(body_text);
  auto message = std::make_unique<Envoy::Http::ResponseMessageImpl>(std::move(response_headers));

  ASSERT(callbacks_ != nullptr);
  callbacks_->onComplete(message.get());
  // callbacks_ = nullptr;
}

FilterConfig::FilterConfig(const HttpFilterDemo& http_filter_demo, const std::string& stats_prefix, Stats::Scope& scope)
  : placeholder_(http_filter_demo.placeholder()), 
    scope_(scope),
    stat_name_pool_(scope_.symbolTable()),
    stats_(generateStats(stats_prefix, scope_)), 
    filter_stat_prefix_(stat_name_pool_.add("http_filter_demo")),
    route_(stat_name_pool_.add("route")),
    count_(stat_name_pool_.add("count")) {}


HttpFilterDemoFilter::HttpFilterDemoFilter(FilterConfigSharedPtr config, ClientPtr&& client) 
  : config_(config), client_(std::move(client)) {}

Http::FilterDataStatus HttpFilterDemoFilter::decodeData(Buffer::Instance& /*data*/, bool /*end_stream*/) {
  // if (!end_stream) {
  //   decoder_callbacks_->addDecodedData(data, false);
  //   return Http::FilterDataStatus::StopIterationAndBuffer;
  // }

  // decoder_callbacks_->addDecodedData(data, false);
  // const Buffer::Instance& decoding_buffer = *decoder_callbacks_->decodingBuffer();
  // auto jsonObj = Json::Nlohmann::Factory::loadFromString(decoding_buffer.toString());
  // if (jsonObj) {
  //   std::string model_ = jsonObj->getString("model");
  // }
  return Http::FilterDataStatus::Continue;
}

Http::FilterHeadersStatus HttpFilterDemoFilter::encodeHeaders(Http::ResponseHeaderMap& headers, bool) {
  headers.remove(Envoy::Http::LowerCaseString("content-length"));
  return Http::FilterHeadersStatus::Continue;
}

Http::FilterDataStatus HttpFilterDemoFilter::encodeData(Buffer::Instance& data, bool end_stream) {
  if (data.length() > 0) {
    ENVOY_LOG(info, "[HttpFilterDemo] receive response body: {}", data.toString());
    chunk_queue_.push_back(std::move(data));
    data.drain(data.length());
  }
  if (!during_call_ && !chunk_queue_.empty()) {
    during_call_ = true;
    client_->call(*this);
  }
  if (end_stream) {
    ENVOY_LOG(info, "[HttpFilterDemo] receive the last chunk {}", data.toString());
    // enter twice will cause crash
    end_stream_ = true;
    return Http::FilterDataStatus::StopIterationAndBuffer;
  }
  return Http::FilterDataStatus::Continue;
}

void HttpFilterDemoFilter::onComplete(const Http::ResponseMessage* response_ptr) {
  ENVOY_LOG(info, "async call response: {}", response_ptr->bodyAsString());
  auto &data = chunk_queue_.front();
  bool end_stream = chunk_queue_.empty() && end_stream_;
  encoder_callbacks_->injectEncodedDataToFilterChain(data, end_stream);
  chunk_queue_.pop_front();
  if (!chunk_queue_.empty()) {
    client_->call(*this);
  } else {
    during_call_ = false;
    if (end_stream_) {
      client_->resetCallbacks();
      encoder_callbacks_->continueEncoding();
    }
  }
}

} // namespace HttpFilterDemo
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
