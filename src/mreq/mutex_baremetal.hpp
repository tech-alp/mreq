#include "mreq/internal/NonCopyable.hpp"

namespace mreq {

struct Mutex : private internal::NonCopyable {
    void lock()   {}
    void unlock() {}
    bool try_lock() { return true; }
    void destroy() {}
};

} // namespace mreq 