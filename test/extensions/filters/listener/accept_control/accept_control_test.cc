#include "envoy/config/core/v3/base.pb.h"
#include "envoy/extensions/filters/listener/accept_control/v3/accept_control.pb.h"

#include "source/common/network/socket_option_impl.h"
#include "source/common/network/utility.h"
#include "source/common/network/listener_filter_buffer_impl.h"
#include "source/extensions/filters/listener/accept_control/accept_control.h"

#include "test/mocks/buffer/mocks.h"
#include "test/mocks/common.h"
#include "test/mocks/network/mocks.h"
#include "test/test_common/printers.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


// using testing::_;
using testing::ByMove;
using testing::NiceMock;
using testing::InSequence;
using testing::Return;
using testing::ReturnRef;

namespace Envoy {
namespace Extensions {
namespace ListenerFilters {
namespace AcceptControl {
namespace {

class AcceptControlTest : public testing::Test, public Event::TestUsingSimulatedTime {
public:
  class ActiveFilter {
  public:
    ActiveFilter(const ConfigSharedPtr& config) : filter_(config) {
      EXPECT_EQ(filter_.maxReadBytes(), 0);
      ON_CALL(cb_, socket()).WillByDefault(ReturnRef(socket_));
      ON_CALL(socket_, ioHandle()).WillByDefault(ReturnRef(io_handle_));
    }

    Network::FilterStatus onAccept() { return filter_.onAccept(cb_); }

    void setAddressToReturn(const std::string& address) {
      socket_.connection_info_provider_->setRemoteAddress(
          Network::Utility::resolveUrl(address));
    }

    void expectIoHandleClose() {
      EXPECT_CALL(io_handle_, close()).WillOnce(Return(ByMove(Api::ioCallUint64ResultNoError())));
    }

  private:
    Filter filter_;
    NiceMock<Network::MockListenerFilterCallbacks> cb_;
    NiceMock<Network::MockConnectionSocket> socket_;
    NiceMock<Network::MockIoHandle> io_handle_;
  };

  void initialize(const std::string& filter_yaml) {
    envoy::extensions::filters::listener::accept_control::v3::AcceptControl proto_config;
    TestUtility::loadFromYaml(filter_yaml, proto_config);
    config_ = std::make_shared<Config>(proto_config, *store_.rootScope());
  }

  std::shared_ptr<Config> config_;
  Stats::TestUtil::TestStore store_;
};

TEST_F(AcceptControlTest, Ipv4AddressAllowHit) {
  initialize(R"EOF(
action: ALLOW
stat_prefix: accept_control
ip_list:
- address_prefix: "127.0.0.1"
  prefix_len: 32
)EOF");

  InSequence s;
  ActiveFilter active_filter(config_);
  active_filter.setAddressToReturn("tcp://127.0.0.1:10000");
  EXPECT_EQ(Network::FilterStatus::Continue, active_filter.onAccept());
  EXPECT_EQ(1, config_->stats().connection_accept_allow_.value());
  EXPECT_EQ(0, config_->stats().connection_accept_deny_.value());
}

TEST_F(AcceptControlTest, Ipv4AddressAllowMiss) {
  initialize(R"EOF(
action: ALLOW
ip_list:
- address_prefix: "127.0.0.1"
  prefix_len: 32
)EOF");

  ActiveFilter active_filter(config_);
  active_filter.setAddressToReturn("tcp://172.0.0.1:10000");
  active_filter.expectIoHandleClose();
  EXPECT_EQ(Network::FilterStatus::StopIteration, active_filter.onAccept());
  EXPECT_EQ(0, config_->stats().connection_accept_allow_.value());
  EXPECT_EQ(1, config_->stats().connection_accept_deny_.value());
}

TEST_F(AcceptControlTest, Ipv4AddressDenyMiss) {
  initialize(R"EOF(
action: DENY
ip_list:
- address_prefix: "127.0.0.1"
  prefix_len: 32
)EOF");

  ActiveFilter active_filter(config_);
  active_filter.setAddressToReturn("tcp://172.0.0.1:10000");
  EXPECT_EQ(Network::FilterStatus::Continue, active_filter.onAccept());
  EXPECT_EQ(1, config_->stats().connection_accept_allow_.value());
  EXPECT_EQ(0, config_->stats().connection_accept_deny_.value());
}

TEST_F(AcceptControlTest, Ipv4AddressDenyHit) {
  initialize(R"EOF(
action: DENY
ip_list:
- address_prefix: "127.0.0.1"
  prefix_len: 32
)EOF");

  ActiveFilter active_filter(config_);
  active_filter.setAddressToReturn("tcp://127.0.0.1:10000");
  active_filter.expectIoHandleClose();
  EXPECT_EQ(Network::FilterStatus::StopIteration, active_filter.onAccept());
  EXPECT_EQ(0, config_->stats().connection_accept_allow_.value());
  EXPECT_EQ(1, config_->stats().connection_accept_deny_.value());
}


} // namespace
} // namespace AcceptControl
} // namespace ListenerFilters
} // namespace Extensions
} // namespace Envoy
