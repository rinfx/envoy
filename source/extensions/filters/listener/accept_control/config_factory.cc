#include "source/extensions/filters/listener/accept_control/config_factory.h"

#include "envoy/extensions/filters/listener/accept_control/v3/accept_control.pb.h"
#include "envoy/extensions/filters/listener/accept_control/v3/accept_control.pb.validate.h"
#include "envoy/registry/registry.h"

#include "source/extensions/filters/listener/accept_control/config.h"
#include "source/extensions/filters/listener/accept_control/accept_control.h"

namespace Envoy {
namespace Extensions {
namespace ListenerFilters {
namespace AcceptControl {

Network::ListenerFilterFactoryCb ConfigFactory::createListenerFilterFactoryFromProto(
    const Protobuf::Message& message,
    const Network::ListenerFilterMatcherSharedPtr& listener_filter_matcher,
    Server::Configuration::ListenerFactoryContext& context) {
  auto proto_config = MessageUtil::downcastAndValidate<
      const envoy::extensions::filters::listener::accept_control::v3::AcceptControl&>(
      message, context.messageValidationVisitor());
  ConfigSharedPtr config = std::make_shared<Config>(proto_config, context.scope());
  return [listener_filter_matcher, config](Network::ListenerFilterManager& filter_manager) -> void {
    filter_manager.addAcceptFilter(listener_filter_matcher,
                                   std::make_unique<Filter>(config));
  };
}

ProtobufTypes::MessagePtr ConfigFactory::createEmptyConfigProto() {
  return std::make_unique<envoy::extensions::filters::listener::accept_control::v3::AcceptControl>();
}
/**
 * Static registration for the accept_control filter. @see RegisterFactory.
 */
REGISTER_FACTORY(ConfigFactory, Server::Configuration::NamedListenerFilterConfigFactory){
    "envoy.listener.accept_control"};

} // namespace AcceptControl
} // namespace ListenerFilters
} // namespace Extensions
} // namespace Envoy
