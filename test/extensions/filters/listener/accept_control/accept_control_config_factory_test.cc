#include "source/extensions/filters/listener/accept_control/config.h"
#include "source/extensions/filters/listener/accept_control/accept_control.h"
#include "source/extensions/filters/listener/accept_control/config_factory.h"

#include "test/mocks/server/listener_factory_context.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::Invoke;
using testing::NiceMock;

namespace Envoy {
namespace Extensions {
namespace ListenerFilters {
namespace AcceptControl {
namespace {

TEST(ConfigFactoryTest, TestCreateFactory) {
  std::string yaml = R"EOF(
action: ALLOW
stat_prefix: accept_control
ip_list:
- address_prefix: 127.0.0.1
  prefix_len: 32
- address_prefix: "::1"
  prefix_len: 64
)EOF";

  ConfigFactory factory;
  ProtobufTypes::MessagePtr proto_config = factory.createEmptyConfigProto();
  TestUtility::loadFromYaml(yaml, *proto_config);

  NiceMock<Server::Configuration::MockListenerFactoryContext> context;

  Network::ListenerFilterFactoryCb cb =
      factory.createListenerFilterFactoryFromProto(*proto_config, nullptr, context);
  Network::MockListenerFilterManager manager;
  Network::ListenerFilterPtr added_filter;
  EXPECT_CALL(manager, addAcceptFilter_(_, _))
      .WillOnce(Invoke([&added_filter](const Network::ListenerFilterMatcherSharedPtr&,
                                       Network::ListenerFilterPtr& filter) {
        added_filter = std::move(filter);
      }));
  cb(manager);

  // Make sure we actually create the correct type!
  EXPECT_NE(dynamic_cast<Filter*>(added_filter.get()), nullptr);
}

} // namespace
} // namespace AcceptControl
} // namespace ListenerFilters
} // namespace Extensions
} // namespace Envoy
