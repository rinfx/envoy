#pragma once

#include "envoy/extensions/filters/listener/accept_control/v3/accept_control.pb.h"

#include "envoy/stats/scope.h"
#include "envoy/stats/stats_macros.h"
#include "source/common/network/cidr_range.h"

namespace Envoy {
namespace Extensions {
namespace ListenerFilters {
namespace AcceptControl {

/**
 * All accept_control limit stats. @see stats_macros.h
 */
#define ALL_ACCEPT_CONTROL_STATS(COUNTER) \
  COUNTER(connection_accept_allow) \
  COUNTER(connection_accept_deny)

/**
 * Struct definition for all accept_control limit stats. @see stats_macros.h
 */
struct AcceptControlStats {
  ALL_ACCEPT_CONTROL_STATS(GENERATE_COUNTER_STRUCT)
};

class Config {
public:
  Config(const envoy::extensions::filters::listener::accept_control::v3::AcceptControl& config, Stats::Scope& scope);
  bool hitList(const Network::Address::Instance& address);
  bool isDenyAction();
  const AcceptControlStats& stats() const { return stats_; }

private:
  void incCounter(const Stats::StatNameVec& names) const {
    const Stats::SymbolTable::StoragePtr stat_name_storage = scope_.symbolTable().join(names);
    scope_.counterFromStatName(Stats::StatName(stat_name_storage.get())).inc();
  }

  static AcceptControlStats generateStats(const std::string& prefix, Stats::Scope& scope) {
    return {ALL_ACCEPT_CONTROL_STATS(POOL_COUNTER_PREFIX(scope, prefix))};
  }

  const AcceptControlStats stats_;
  Stats::Scope& scope_;
  Stats::StatNamePool stat_name_pool_;
  const Stats::StatName filter_stat_prefix_;
  envoy::extensions::filters::listener::accept_control::v3::AcceptControl::Action action_;
  std::vector<std::pair<Network::Address::CidrRange, Stats::StatName>> ip_list_;
};

using ConfigSharedPtr = std::shared_ptr<Config>;

} // namespace AcceptControl
} // namespace ListenerFilters
} // namespace Extensions
} // namespace Envoy
