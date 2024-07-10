#pragma once

#include "envoy/network/address.h"
#include "envoy/network/filter.h"

#include "source/common/common/logger.h"
#include "source/common/network/cidr_range.h"
#include "source/extensions/filters/listener/accept_control/config.h"

namespace Envoy {
namespace Extensions {
namespace ListenerFilters {
namespace AcceptControl {

/**
 * Implements the Accept Control network filter.
 */
class Filter : public Network::ListenerFilter, Logger::Loggable<Logger::Id::filter> {
public:
  Filter(const ConfigSharedPtr& config) : config_(config) {}

  // Network::ListenerFilter
  Network::FilterStatus onAccept(Network::ListenerFilterCallbacks& cb) override;

  size_t maxReadBytes() const override { return 0; }

  Network::FilterStatus onData(Network::ListenerFilterBuffer&) override {
    return Network::FilterStatus::Continue;
  };

private:
  ConfigSharedPtr config_;
};

} // namespace AcceptControl
} // namespace ListenerFilters
} // namespace Extensions
} // namespace Envoy
