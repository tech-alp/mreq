#ifndef MREQ_NONCOPYABLE_HPP
#define MREQ_NONCOPYABLE_HPP

namespace mreq {
namespace internal {

class NonCopyable {
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

} // namespace internal
} // namespace mreq

#endif // MREQ_NONCOPYABLE_HPP 