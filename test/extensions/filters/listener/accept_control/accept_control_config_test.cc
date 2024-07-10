#include <memory>
#include <numeric>

#include "envoy/extensions/filters/listener/accept_control/v3/accept_control.pb.h"

#include "source/extensions/filters/listener/accept_control/config.h"
#include "source/common/network/cidr_range.h"
#include "source/common/network/address_impl.h"

#include "test/test_common/utility.h"
#include "test/mocks/stats/mocks.h"
#include "gtest/gtest.h"

using testing::NiceMock;

namespace Envoy {
namespace Extensions {
namespace ListenerFilters {
namespace AcceptControl {
namespace {

// In keeping with the class under test, it would have made sense to call this ConfigTest. However,
// when running coverage tests, that conflicts with tests elsewhere in the codebase.
class AcceptControlConfigTest : public testing::Test {
public:
  Config makeConfigFromProto(const envoy::extensions::filters::listener::accept_control::v3::AcceptControl& proto_config) {
    NiceMock<Stats::MockStore> store;
    return {proto_config, *store.rootScope()};
  }
};

TEST_F(AcceptControlConfigTest, TestActionDeny) {
  envoy::extensions::filters::listener::accept_control::v3::AcceptControl config_proto;
  config_proto.set_action(envoy::extensions::filters::listener::accept_control::v3::AcceptControl::Action::AcceptControl_Action_DENY);
  auto config = makeConfigFromProto(config_proto);

  EXPECT_TRUE(config.isDenyAction());
}

TEST_F(AcceptControlConfigTest, TestActionAllow) {
  envoy::extensions::filters::listener::accept_control::v3::AcceptControl config_proto;
  config_proto.set_action(envoy::extensions::filters::listener::accept_control::v3::AcceptControl::Action::AcceptControl_Action_ALLOW);
  auto config = makeConfigFromProto(config_proto);

  EXPECT_FALSE(config.isDenyAction());
}

TEST_F(AcceptControlConfigTest, TestIpv4List) {
  envoy::extensions::filters::listener::accept_control::v3::AcceptControl config_proto;

  const std::string yaml = R"EOF(
action: ALLOW
ip_list:
- address_prefix: 127.0.0.1
  prefix_len: 32
)EOF";

  TestUtility::loadFromYaml(yaml, config_proto);

  auto config = makeConfigFromProto(config_proto);
  EXPECT_EQ(1, config_proto.ip_list_size());

  Network::Address::Ipv4Instance local_address_ipv4_hit_ { "127.0.0.1", 10000 };
  Network::Address::Ipv4Instance local_address_ipv4_not_hit_ { "121.43.135.220", 10000 };

  EXPECT_TRUE(config.hitList(local_address_ipv4_hit_));
  EXPECT_FALSE(config.hitList(local_address_ipv4_not_hit_));
}

TEST_F(AcceptControlConfigTest, TestIpv6List) {
  envoy::extensions::filters::listener::accept_control::v3::AcceptControl config_proto;

  const std::string yaml = R"EOF(
action: ALLOW
ip_list:
- address_prefix: "::1"
  prefix_len: 64
)EOF";

  TestUtility::loadFromYaml(yaml, config_proto);

  auto config = makeConfigFromProto(config_proto);
  EXPECT_EQ(1, config_proto.ip_list_size());

  Network::Address::Ipv6Instance local_address_ipv6_hit_ { "::1", 10000 };
  Network::Address::Ipv6Instance local_address_ipv6_not_hit_ { "fe80::871:35fa:5f99:7bd", 10000 };

  EXPECT_TRUE(config.hitList(local_address_ipv6_hit_));
  EXPECT_FALSE(config.hitList(local_address_ipv6_not_hit_));
}

TEST_F(AcceptControlConfigTest, TestIpv4Ipv6List) {
  envoy::extensions::filters::listener::accept_control::v3::AcceptControl config_proto;

  const std::string yaml = R"EOF(
action: ALLOW
ip_list:
- address_prefix: 127.0.0.1
  prefix_len: 32
- address_prefix: "::1"
  prefix_len: 64
)EOF";

  TestUtility::loadFromYaml(yaml, config_proto);

  auto config = makeConfigFromProto(config_proto);
  EXPECT_EQ(2, config_proto.ip_list_size());

  Network::Address::Ipv4Instance local_address_ipv4_hit_ { "127.0.0.1", 10000 };
  Network::Address::Ipv4Instance local_address_ipv4_not_hit_ { "121.43.135.220", 10000 };
  Network::Address::Ipv6Instance local_address_ipv6_hit_ { "::1", 10000 };
  Network::Address::Ipv6Instance local_address_ipv6_not_hit_ { "fe80::871:35fa:5f99:7bd", 10000 };

  EXPECT_TRUE(config.hitList(local_address_ipv4_hit_));
  EXPECT_TRUE(config.hitList(local_address_ipv6_hit_));
  EXPECT_FALSE(config.hitList(local_address_ipv4_not_hit_));
  EXPECT_FALSE(config.hitList(local_address_ipv6_not_hit_));
}

} // namespace
} // namespace AcceptControl
} // namespace ListenerFilters
} // namespace Extensions
} // namespace Envoy
