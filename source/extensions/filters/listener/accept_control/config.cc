#include "source/extensions/filters/listener/accept_control/config.h"

#include "envoy/extensions/filters/listener/accept_control/v3/accept_control.pb.h"

namespace Envoy {
namespace Extensions {
namespace ListenerFilters {
namespace AcceptControl {

Config::Config(const envoy::extensions::filters::listener::accept_control::v3::AcceptControl& config, Stats::Scope& scope)
  : stats_(generateStats(config.stat_prefix(), scope)), 
    scope_(scope), 
    stat_name_pool_(scope.symbolTable()),
    filter_stat_prefix_(stat_name_pool_.add("accept_control_hit")),
    action_(config.action()) {
        for (const auto& entry: config.ip_list()) {
            auto cidr_entry = Network::Address::CidrRange::create(entry);
            ip_list_.push_back(std::make_pair(cidr_entry, stat_name_pool_.add(cidr_entry.asString())));
        }
    }

bool Config::hitList(const Network::Address::Instance& address) {
    for (const auto &entry: ip_list_) {
        if(entry.first.isInRange(address)) {
            incCounter({filter_stat_prefix_, entry.second});
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
