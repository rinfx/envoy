#include "source/extensions/filters/listener/accept_control/config.h"

#include "envoy/extensions/filters/listener/accept_control/v3/accept_control.pb.h"

namespace Envoy {
namespace Extensions {
namespace ListenerFilters {
namespace AcceptControl {

Config::Config(const envoy::extensions::filters::listener::accept_control::v3::AcceptControl& config, Stats::Scope& scope)
    : stats_(generateStats(config.stat_prefix(), scope)), action_(config.action()) {
        for (const auto& entry: config.ip_list()) {
            ip_list_.push_back(Network::Address::CidrRange::create(entry));
        }
    }

bool Config::hitList(const Network::Address::Instance& address) {
    for (const auto &entry: ip_list_) {
        if(entry.isInRange(address)) {
            return true;
        }
    }
    return false;
}

bool Config::isDenyAction() {
    return action_==envoy::extensions::filters::listener::accept_control::v3::AcceptControl::Action::AcceptControl_Action_DENY; 
}

} // namespace AcceptControl
} // namespace ListenerFilters
} // namespace Extensions
} // namespace Envoy
