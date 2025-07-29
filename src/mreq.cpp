#include "mreq/mreq.hpp"
#include "mreq/topic.hpp"
#include <optional>

namespace mreq {

void init() {

}

std::optional<Token> subscribe(const mreq_metadata* metadata) {
    if (!metadata) return std::nullopt;
    auto topic = TopicRegistry::instance().find(metadata);
    if (topic) {
        return topic->subscribe_fn();
    }
    return std::nullopt;
}

bool check(const mreq_metadata* metadata, Token token) {
    if (!metadata) return false;
    auto topic = TopicRegistry::instance().find(metadata);
    if (topic) {
        return topic->check_fn(token);
    }
    return false;
}

void unsubscribe(const mreq_metadata* metadata, Token token) {
    if (!metadata) return;
    auto topic = TopicRegistry::instance().find(metadata);
    if (topic) {
        topic->unsubscribe_fn(token);
    }
}

} // namespace mreq
