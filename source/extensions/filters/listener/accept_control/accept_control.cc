#include "source/extensions/filters/listener/accept_control/accept_control.h"

#include "envoy/buffer/buffer.h"
#include "envoy/network/connection.h"

#include "source/common/common/assert.h"

namespace Envoy {
namespace Extensions {
namespace ListenerFilters {
namespace AcceptControl {

Network::FilterStatus Filter::onAccept(Network::ListenerFilterCallbacks& cb) {
  if (config_->isDenyAction()) {
    if (config_->hitList(*cb.socket().connectionInfoProvider().remoteAddress())) {
      ENVOY_LOG(debug, "Connection accept deny. remote address: {}",
              cb.socket().connectionInfoProvider().remoteAddress()->asString());
      config_->stats().connection_accept_deny_.inc();
      cb.socket().ioHandle().close();
      return Network::FilterStatus::StopIteration;
    } else {
      config_->stats().connection_accept_allow_.inc();
      return Network::FilterStatus::Continue;
    }
  } else {
    if (config_->hitList(*cb.socket().connectionInfoProvider().remoteAddress())) {
      config_->stats().connection_accept_allow_.inc();
      return Network::FilterStatus::Continue;
    } else {
      ENVOY_LOG(debug, "Connection accept deny. remote address: {}",
              cb.socket().connectionInfoProvider().remoteAddress()->asString());
      config_->stats().connection_accept_deny_.inc();
      cb.socket().ioHandle().close();
      return Network::FilterStatus::StopIteration;
    }
  }
}

} // namespace AcceptControl
} // namespace ListenerFilters
} // namespace Extensions
} // namespace Envoy
